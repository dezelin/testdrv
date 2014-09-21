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

#ifndef __DEVICE_H
#define __DEVICE_H

/* Used for logging */
#define CLASS_NAME          "testdrv"

#define DEVICE_AUTHOR       "Aleksandar Dezelin"
#define DEVICE_DESCRIPTION  "Test character device"
#define DEVICE_LICENSE      "GPL"
#define DEVICE_VERSION      "0.0.1"

#define DEVICE_MAJOR        0

#define MAX_BUFFER_SIZE     (1 << 20)


#endif /* __DEVICE_H */

