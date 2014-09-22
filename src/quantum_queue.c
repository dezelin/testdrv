/*
 * This file is part of testdrv.
 *
 * testdrv is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * testdrv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with testdrv. If not, see <http://www.gnu.org/licenses/>.
 */

#include "quantum_queue.h"

#include <linux/errno.h>
#include <linux/slab.h>


int quantum_queue_init(struct quantum_queue *qq)
{
    qq->size = 0;
    qq->quantums = NULL;
    return 0;
}

void quantum_queue_clear(struct quantum_queue *qq)
{
    struct quantum *q, *tmp;
    struct quantum *quantums = qq->quantums;

    if (!quantums)
        return;

    list_for_each_entry_safe(q, tmp, &quantums->list, list) {
        qq->size -= q->size;
        kfree(q->buffer);
        list_del(&q->list);
        kfree(q);
    } 

    kfree(qq->quantums);
    qq->size = 0;
    qq->quantums = NULL;
}

void quantum_queue_destroy(struct quantum_queue *qq)
{
    quantum_queue_clear(qq);
}

int quantum_queue_get_size(struct quantum_queue *qq)
{
    return qq->size;
}

void quantum_queue_push(struct quantum_queue *qq, struct quantum *q)
{
    if (!qq->quantums) {
        qq->quantums = kmalloc(sizeof(struct quantum), GFP_KERNEL);
        if (!qq->quantums)
            return;

        INIT_LIST_HEAD(&qq->quantums->list);
    }

    INIT_LIST_HEAD(&q->list);
    list_add_tail(&q->list, &qq->quantums->list);
    qq->size += q->size;
}

struct quantum* quantum_queue_pop(struct quantum_queue *qq)
{
    struct quantum *q;

    if (!qq->quantums)
        return NULL;

    if ((q = list_first_entry_or_null(&qq->quantums->list, struct quantum, 
                    list)) == NULL)
        return NULL;

    list_del(&q->list);
    qq->size -= q->size;
    return q;
}

