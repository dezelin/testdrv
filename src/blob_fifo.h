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

#ifndef __BLOB_FIFOH
#define __BLOB_FIFOH

#include <linux/list.h>

struct blob {
    char *buffer;
    int size;
    struct list_head list;
};

struct blob_fifo {
   int size;
   struct blob *blobs;  
};

int blob_fifo_init(struct blob_fifo *fifo);
void blob_fifo_clear(struct blob_fifo *fifo);
void blob_fifo_destroy(struct blob_fifo *fifo);

int blob_fifo_size(struct blob_fifo* fifo);
void blob_fifo_push(struct blob_fifo *fifo, struct blob *b);
struct blob* blob_fifo_pop(struct blob_fifo *fifo);

#endif /* __BLOB_FIFOH */

