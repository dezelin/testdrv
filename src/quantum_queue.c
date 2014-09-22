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

#include "log.h"
#include "quantum_queue.h"

#include <linux/errno.h>
#include <linux/slab.h>


int quantum_queue_init(struct quantum_queue *qq)
{
    qq->size = 0;
    qq->quantums = NULL;

    dbg("quantum queue %p initialized, size %d\n", qq, qq->size);
    return 0;
}

void quantum_queue_clear(struct quantum_queue *qq)
{
    struct quantum *q, *tmp;
    struct quantum *quantums = qq->quantums;

    dbg("quantum queue %p, size %d\n", qq, qq->size);

    if (!quantums)
        return;

    list_for_each_entry_safe(q, tmp, &quantums->list, list) {
        dbg("quantum %p deallocated, size %d\n", q, q->size);
        qq->size -= q->size;
        kfree(q->buffer);
        list_del(&q->list);
        kfree(q);
    } 

    kfree(qq->quantums);
    qq->size = 0;
    qq->quantums = NULL;
    dbg("quantum queue %p cleared, size %d\n", qq, qq->size);
}

void quantum_queue_destroy(struct quantum_queue *qq)
{
    quantum_queue_clear(qq);
}

int quantum_queue_get_size(struct quantum_queue *qq)
{
    dbg("quantum queue %p, size %d\n", qq, qq->size);
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
    
    dbg("quantum queue %p, size %d\n", qq, qq->size);
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

    dbg("quantum queue %p, size %d\n", qq, qq->size);
    return q;
}

extern struct quantum* quantum_queue_pop_buff(struct quantum_queue *qq, 
        size_t count)
{
    int rest, total = 0;
    struct quantum *q, *qn, *tmp;
    struct quantum *quantums = qq->quantums;

    if (!quantums)
        return NULL;

    if ((count = min(count, (size_t)qq->size)) == 0)
        return NULL;
    
    if ((qn = quantum_alloc(count)) == NULL)
        return NULL;

    dbg("count = %li\n", count);
    
    list_for_each_entry_safe(q, tmp, &quantums->list, list) {
        if ((rest = (count - total)) < 1)
            break;
        
        dbg("quantum %p, size %d\n", q, q->size);
        dbg("rest = %d\n", rest);
        
        if (q->size <= rest) {
            memcpy(qn->buffer + total, q->buffer, q->size);
            total += q->size;
            qq->size -= q->size;
            list_del(&q->list);
            quantum_dealloc(q);
        } else {
            q->size -= rest;
            qq->size -= rest;
            memcpy(qn->buffer + total, q->buffer, rest);
            memcpy(q->buffer, q->buffer + rest, q->size);
            /* We are shrinking buffer - no need to check */
            q->buffer = krealloc(q->buffer, q->size, GFP_KERNEL);
            total += rest;
        }
    }
   
    dbg("quantum queue %p, size = %li\n", qq, (size_t)qq->size); 
    return qn;
}

struct quantum* quantum_alloc(size_t size)
{
    struct quantum *q;

    if ((q = kmalloc(sizeof(struct quantum), GFP_KERNEL)) == NULL)
        return NULL;

    if ((q->buffer = kmalloc(size, GFP_KERNEL)) == NULL) {
        kfree(q);
        return NULL;
    }

    q->size = size;

    dbg("quantum %p allocated, size %d\n", q, q->size);
    return q;
}

void quantum_dealloc(struct quantum *q)
{
    dbg("quantum %p deallocated, size %d\n", q, q->size);

    kfree(q->buffer);
    kfree(q);
}

