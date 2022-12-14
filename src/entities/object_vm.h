/*
 * object_vm.h - virtual machine of the objects
 * Copyright (C) 2010  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _OBJECT_VM_H
#define _OBJECT_VM_H

#include "enemy.h"
#include "object_decorators/base/objectmachine.h"

/* an objectvm_t is a finite state machine.
   Every state has a name an can be decorated
   (in terms of the Decorator Design Pattern) */

typedef struct objectvm_t objectvm_t;

/* public methods */

objectvm_t* objectvm_create(enemy_t* owner); /* creates a new virtual machine */
objectvm_t* objectvm_destroy(objectvm_t* vm); /* destroys an existing VM */
objectmachine_t** objectvm_get_reference_to_current_state(objectvm_t* vm); /* returns a reference to the current state */
void objectvm_create_state(objectvm_t* vm, const char *name); /* you have to create a state before you can use it */
void objectvm_set_current_state(objectvm_t* vm, const char *name); /* sets the current state */

#endif
