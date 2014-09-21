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
#include "log.h"
#include "quantum_queue.h"

#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

static int device_major = DEVICE_MAJOR;
static int device_minor = 0;
module_param(device_major, int, S_IRUGO);

static int device_buffer_size = MAX_BUFFER_SIZE;
module_param(device_buffer_size, int, S_IRUGO);

MODULE_AUTHOR(DEVICE_AUTHOR);
MODULE_DESCRIPTION(DEVICE_DESCRIPTION);
MODULE_LICENSE(DEVICE_LICENSE);
MODULE_VERSION(DEVICE_VERSION);


static struct testdrv_device {
    struct quantum_queue *qq;
    int offset;
    int size;
    struct semaphore sem;
    struct cdev cdev;
} testdrv_dev;


static struct file_operations testdrv_fops = {
    .owner = THIS_MODULE
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

