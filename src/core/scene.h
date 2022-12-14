/*
 * scene.h - scene management
 * Copyright (C) 2008-2010  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#ifndef _SCENE_H
#define _SCENE_H

/* Scene struct */
typedef struct {
    void (*init)();
    void (*update)();
    void (*render)();
    void (*release)();
} scene_t;

/* Scene stack */
/* this is used with the storyboard module (see storyboard.h) */
void scenestack_init();
void scenestack_release();
void scenestack_push(scene_t *scn);
void scenestack_pop();
scene_t *scenestack_top();
int scenestack_empty();

#endif
