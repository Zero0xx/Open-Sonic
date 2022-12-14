/*
 * change_closest_object_state.h - Changes the state of the closest object with a given name
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

#ifndef _OD_CHANGECLOSESTOBJECTSTATE_H
#define _OD_CHANGECLOSESTOBJECTSTATE_H

#include "base/objectdecorator.h"

objectmachine_t* objectdecorator_changeclosestobjectstate_new(objectmachine_t *decorated_machine, const char* object_name, const char *new_state_name);

#endif

