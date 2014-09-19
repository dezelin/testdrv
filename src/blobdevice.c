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

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int blobdevice_init(void)
{
    printk(KERN_ALERT "Hello, world!\n");
    return 0;
}

static void blobdevice_exit(void)
{
    printk(KERN_ALERT "Goodbye, cruel world!\n");
}

module_init(blobdevice_init);
module_exit(blobdevice_exit);

