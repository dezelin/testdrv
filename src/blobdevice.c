/*
 * This file is part of blobdevice.
 *
 * blobdevice is free software: you can redistribute it and/or modify
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

#include "blobdevice.h"

#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>

static int blobdevice_major = BLOBDEVICE_MAJOR;
static int blobdevice_minor = 0;
module_param(blobdevice_major, int, S_IRUGO);

static int blobdevice_buffer_size = MAX_BUFFER_SIZE;
module_param(blobdevice_buffer_size, int, S_IRUGO);

MODULE_AUTHOR(BLOBDEVICE_AUTHOR);
MODULE_DESCRIPTION(BLOBDEVICE_DESCRIPTION);
MODULE_LICENSE(BLOBDEVICE_LICENSE);
MODULE_VERSION(BLOBDEVICE_VERSION);

static struct blobdevice_device {
    char *buffer;
    int offset;
    struct semaphore sem;
    struct cdev cdev;
} blobdevice_dev;

static struct file_operations blobdevice_fops = {
    .owner = THIS_MODULE
};

static void blobdevice_device_clean(struct blobdevice_device *dev)
{
    cdev_del(&dev->cdev);
    kfree(dev->buffer);
}

static int blobdevice_device_init(struct blobdevice_device *dev) 
{
    dev->buffer = 0;
    dev->offset = 0;
    sema_init(&dev->sem, 1);
    
    cdev_init(&dev->cdev, &blobdevice_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &blobdevice_fops;

    return cdev_add(&dev->cdev, MKDEV(blobdevice_major, blobdevice_minor), 1);
}

static int __init blobdevice_init(void)
{
    dev_t dev;
    int result;

    printk(KERN_ALERT "blobdevice: module loaded\n");
    
    if (blobdevice_major) {
        dev = MKDEV(blobdevice_major, blobdevice_minor);
        result = register_chrdev_region(dev, 1, "blobdevice"); 
    } else {
        result = alloc_chrdev_region(&dev, blobdevice_minor, 1, "blobdevice");
        blobdevice_major = MAJOR(dev);
    }

    if (result < 0) {
        printk(KERN_ALERT "blockdevice: can't allocate device major: %d\n", result);
        return result;
    }

    if ((result = blobdevice_device_init(&blobdevice_dev)) < 0) {
        printk(KERN_ALERT "blockdevice: error adding device: %d\n", result);
        return result;
    }

    return 0;
}

static void __exit blobdevice_exit(void)
{
    blobdevice_device_clean(&blobdevice_dev);
    unregister_chrdev_region(MKDEV(blobdevice_major, blobdevice_minor), 1);
    printk(KERN_ALERT "blobdevice: module unloaded\n");
}

module_init(blobdevice_init);
module_exit(blobdevice_exit);

