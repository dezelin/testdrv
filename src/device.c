/*
 * This file is part of test_drv.
 *
 * test_drv is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with blobdevice. If not, see <http://www.gnu.org/licenses/>.
 */

#include "device.h"
#include "quantum_queue.h"

#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/slab.h>

static int device_major = DEVICE_MAJOR;
static int device_minor = 0;
module_param(device_major, int, S_IRUGO);

static int device_buffer_size = MAX_BUFFER_SIZE;
module_param(device_buffer_size, int, S_IRUGO|S_IWUSR);

/* extern */
int debug = 1;
module_param(debug, int, S_IRUGO|S_IWUSR);

#include "log.h"

MODULE_AUTHOR(DEVICE_AUTHOR);
MODULE_DESCRIPTION(DEVICE_DESCRIPTION);
MODULE_LICENSE(DEVICE_LICENSE);
MODULE_VERSION(DEVICE_VERSION);


static struct testdrv_device {
    struct quantum_queue *qq;
    int offset;
    int size;
    struct semaphore sem;
    wait_queue_head_t readq, writeq;
    struct cdev cdev;
} testdrv_dev;



static ssize_t testdrv_device_read(struct file *filp, char __user *buf, 
        size_t count, loff_t *f_pos)
{
    struct quantum *q;
    struct testdrv_device *dev = filp->private_data;

    dbg("Request to read %li bytes\n", count);
    if (count == 0)
        return 0;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    while(quantum_queue_get_size(dev->qq) == 0) {
        /* Nothing to read */
        up(&dev->sem);
        if (filp->f_flags & O_NONBLOCK)
            return -EAGAIN;

        dbg("\"%s\" reading: going to sleep\n", current->comm);
        if (wait_event_interruptible(dev->readq, 
                    (quantum_queue_get_size(dev->qq) > 0)))
            return -ERESTARTSYS;

        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
    }

    /* Pop count bytes from the quantum queue */
    count = min(count, (size_t)dev->qq->size);
    if ((q = quantum_queue_pop_buff(dev->qq, count)) == NULL) {
        up(&dev->sem);
        return -EFAULT;
    }

    dbg("count = %li\n", count);
    count = min(count, (size_t)q->size);

    /* Copy data to user space */
    if (copy_to_user(buf, q->buffer, count)) {
        up(&dev->sem);
        return -EFAULT;
    }

    /* Destroy allocated quantum */
    quantum_dealloc(q);
    up(&dev->sem);

    /* Wake up writers */
    wake_up_interruptible(&dev->writeq);

    dbg("\%s\" read %li bytes\n", current->comm, (long)count);

    return count;
}

static ssize_t testdrv_device_write(struct file *filp, const char __user *buf, 
        size_t count, loff_t *f_pos)
{
    struct quantum *q;
    struct testdrv_device *dev = filp->private_data;

    if (count == 0)
        return 0;
    
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    /* Sleep while space is not available */
    while(quantum_queue_get_size(dev->qq) >= device_buffer_size) {
        DEFINE_WAIT(wait);

        up(&dev->sem);
        if (filp->f_flags & O_NONBLOCK)
            return -EAGAIN;

        dbg("\"%s\" writing: going to sleep\n", current->comm);
    
        prepare_to_wait(&dev->writeq, &wait, TASK_INTERRUPTIBLE);
        if (quantum_queue_get_size(dev->qq) >= device_buffer_size)
            schedule();

        finish_wait(&dev->writeq, &wait);
        if (signal_pending(current))
            return -ERESTARTSYS;

        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
    }

    /* Allocate new quantum for data */
    count = min(count, (size_t)(device_buffer_size - 
                quantum_queue_get_size(dev->qq)));
    if ((q = quantum_alloc(count)) == NULL) {
        up(&dev->sem);
        return -EFAULT;
    }

    dbg("Going to accept %li bytes to %p from %p\n", (long)count, q->buffer, buf);

    /* Copy data from user space to the quantum */
    if (copy_from_user(q->buffer, buf, count)) {
        up(&dev->sem);
        return -EFAULT;
    }

    /* Add the quantum to the queue */
    quantum_queue_push(dev->qq, q);
    up(&dev->sem);
    
    /* Awake readers */
    wake_up_interruptible(&dev->readq);
    
    dbg("\"%s\" wrote %li bytes\n", current->comm, (long)count);

    return count;
}

static int testdrv_device_open(struct inode *inode, struct file *filp)
{
    filp->private_data = container_of(inode->i_cdev, struct testdrv_device, cdev);
    return 0;
}

static int testdrv_device_release(struct inode *inode, struct file *filp)
{
    return 0;
}


static struct file_operations testdrv_fops = {
    .owner      = THIS_MODULE,
    .read       = testdrv_device_read,
    .write      = testdrv_device_write,
    .open       = testdrv_device_open,
    .release    = testdrv_device_release,
};


static void testdrv_device_clean(struct testdrv_device *dev)
{
    cdev_del(&dev->cdev);
    quantum_queue_destroy(dev->qq); 
}

static int testdrv_device_init(struct testdrv_device *dev) 
{
    int res;

    if ((dev->qq = kmalloc(sizeof(struct quantum_queue), GFP_KERNEL)) == NULL)
        return -ENOMEM;

    if ((res = quantum_queue_init(dev->qq)) < 0)
        return res; 

    dev->size = 0;
    dev->offset = 0;

    sema_init(&dev->sem, 1);
    init_waitqueue_head(&dev->readq);
    init_waitqueue_head(&dev->writeq);
    
    cdev_init(&dev->cdev, &testdrv_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &testdrv_fops;

    return cdev_add(&dev->cdev, MKDEV(device_major, device_minor), 1);
}

static int __init testdrv_init(void)
{
    dev_t dev;
    int result;

    info("testdrv: module loaded\n");
    
    if (device_major) {
        dev = MKDEV(device_major, device_minor);
        result = register_chrdev_region(dev, 1, CLASS_NAME); 
    } else {
        result = alloc_chrdev_region(&dev, device_minor, 1, CLASS_NAME);
        device_major = MAJOR(dev);
    }

    if (result < 0) {
        err("can't allocate device major: %d\n", result);
        return result;
    }

    if ((result = testdrv_device_init(&testdrv_dev)) < 0) {
        err("error adding device: %d\n", result);
        return result;
    }

    return 0;
}

static void __exit testdrv_exit(void)
{
    testdrv_device_clean(&testdrv_dev);
    unregister_chrdev_region(MKDEV(device_major, device_minor), 1);
    info("module unloaded\n");
}

module_init(testdrv_init);
module_exit(testdrv_exit);

