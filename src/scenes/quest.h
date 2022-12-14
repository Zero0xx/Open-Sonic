/*
 * quest.h - quest scene
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

#ifndef _QUESTSCENE_H
#define _QUESTSCENE_H

/*
   There is only one quest running at a time.

   This is actually a "mock" scene that just
   dispatches the player to the correct levels.
   How does it know the correct levels? By
   looking at the quest_t* structure, of course.

   quest_t* contains all the data relevant to
   a quest (including name, author and list of
   levels), but it does nothing for itself.
   (see ../core/quest.h)
*/

#include "../core/quest.h"

/* public scene functions */
/* call quest_run() before pushing this scene into the stack! */
void quest_init();
void quest_update();
void quest_render();
void quest_release();

/* specific methods */
void quest_run(quest_t *qst, int standalone_quest); /* executes the given quest */
void quest_setlevel(int lev); /* jumps to the given level (0..n-1) */
void quest_abort(); /* aborts the current quest */
const char *quest_getname(); /* returns the name of the current quest */

/* quest values */
typedef enum questvalue_t {
    QUESTVALUE_TOTALTIME,   /* total quest time, in seconds */
    QUESTVALUE_BIGRINGS,    /* how many big rings has the player got so far? */
    QUESTVALUE_GLASSES      /* how many magic glasses has the player got so far? */
} questvalue_t;

void quest_setvalue(questvalue_t key, float value);
float quest_getvalue(questvalue_t key);

#endif
