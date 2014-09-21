/*
 * This file is part of testdrv.
 *
 * testdrv is free software: you can redistribute it and/or modify
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
 * along with testdrv. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __QUANTUM_QUEUE_H
#define __QUANTUM_QUEUE_H

#include <linux/list.h>

struct quantum {
    char *buffer;
    int size;
    struct list_head list;
};

struct quantum_queue {
   int size;
   struct quantum *quantums;  
};

extern int quantum_queue_init(struct quantum_queue *qq);
extern void quantum_queue_clear(struct quantum_queue *qq);
extern void quantum_queue_destroy(struct quantum_queue *qq);

int quantum_queue_size(struct quantum_queue *qq);
void quantum_queue_push(struct quantum_queue *qq, struct quantum *q);
struct quantum* quantum_queue_pop(struct quantum_queue *qq);

#endif /* __QUANTUM_QUEUE_H */

