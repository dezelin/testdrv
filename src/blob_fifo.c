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

#include "blob_fifo.h"

#include <linux/errno.h>
#include <linux/slab.h>


int blob_fifo_init(struct blob_fifo *fifo)
{
    fifo->size = 0;
    fifo->blobs = NULL;
    return 0;
}

void blob_fifo_clear(struct blob_fifo *fifo)
{
    struct blob *blob, *tmp;
    struct blob *blob_list = fifo->blobs;
    list_for_each_entry_safe(blob, tmp, &blob_list->list, list) {
        fifo->size -= blob->size;
        kfree(blob->buffer);
        list_del(&blob->list);
        kfree(blob);
    } 

    kfree(fifo->blobs);
    fifo->size = 0;
    fifo->blobs = NULL;
}

void blob_fifo_destroy(struct blob_fifo *fifo)
{
    blob_fifo_clear(fifo);
}

int blob_fifo_get_size(struct blob_fifo *fifo)
{
    return fifo->size;
}

void blob_fifo_push(struct blob_fifo *fifo, struct blob *b)
{
    if (!fifo->blobs) {
        fifo->blobs = kmalloc(sizeof(struct blob), GFP_KERNEL);
        if (!fifo->blobs)
            return;

        INIT_LIST_HEAD(&fifo->blobs->list);
    }

    INIT_LIST_HEAD(&b->list);
    list_add_tail(&b->list, &fifo->blobs->list);
    fifo->size += b->size;
}

struct blob* blob_fifo_pop(struct blob_fifo *fifo)
{
    struct blob *head;

    if (!fifo->blobs)
        return NULL;

    head = list_first_entry_or_null(&fifo->blobs->list, struct blob, list);
    if (!head)
        return NULL;

    list_del(&head->list);
    fifo->size -= head->size;
    return head;
}

