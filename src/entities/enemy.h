/*
 * enemy.h - baddies
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


#ifndef _ENEMY_H
#define _ENEMY_H

#include "../core/v2d.h"

/* forward declarations */
struct actor_t;
struct player_t;
struct brick_list_t;
struct item_list_t;
struct objectvm_t;
struct object_child_list_t;
typedef struct enemy_t enemy_t;
typedef struct enemy_list_t enemy_list_t;
typedef enum enemystate_t enemystate_t;

/* enemy state */
enum enemystate_t {
    ES_IDLE,            /* idle: default state */
    ES_DEAD             /* dead objects are automatically removed from the object list */
};

/* enemy class */
struct enemy_t {
    /* public attributes */
    char *name; /* name */
    struct actor_t *actor; /* actor */
    enemystate_t state; /* state */
    int created_from_editor; /* was this created from the level editor? */

    int preserve; /* should not be removed if far from play area? */
    int obstacle; /* does this behave like an obstacle brick? */
    int obstacle_angle; /* if this is an obstacle, what is my angle a, 0 <= a < 360? */
    int always_active; /* is this object always active, even if it's far away from the camera? */
    int hide_unless_in_editor_mode; /* this object will be displayed only in the level editor */
    struct objectvm_t *vm; /* virtual machine (programming related to objects) */

    /* private attributes */
    enemy_t *parent; /* if someone has created me, who is that? */
    struct object_children_t *children; /* I have created my children */
    struct player_t *observed_player; /* NULL iff I'm observing the active player */
};

/* linked list of enemies */
struct enemy_list_t {
    enemy_t *data;
    enemy_list_t *next;
};


/* ------ public class methods ---------- */

/* initializes this module */
void objects_init();

/* releases this module */
void objects_release();

/* returns an array v[0..n-1] of available object names */
const char** objects_get_list_of_names(int *n);




/* ------ public instance methods: generic routines ------- */

/* creates a new enemy instance */
enemy_t *enemy_create(const char *name);

/* destroys an existing enemy instance */
enemy_t *enemy_destroy(enemy_t *enemy);

/* updates an enemy */
void enemy_update(enemy_t *enemy, struct player_t **team, int team_size, struct brick_list_t *brick_list, struct item_list_t *item_list, struct enemy_list_t *object_list);

/* renders an enemy */
void enemy_render(enemy_t *enemy, v2d_t camera_position);





/* ------ public instance methods: programming-specific routines ------- */

/* finds the parent */
enemy_t *enemy_get_parent(enemy_t *enemy);

/* finds a child */
enemy_t *enemy_get_child(enemy_t *enemy, const char *child_name);

/* adds a child */
void enemy_add_child(enemy_t *enemy, const char *child_name, enemy_t *child);

/* removes a child from this object (the child is not deleted, though) */
void enemy_remove_child(enemy_t *enemy, enemy_t *child);

/* returns the observed player */
struct player_t *enemy_get_observed_player(enemy_t *enemy);

/* observes a new player */
void enemy_observe_player(enemy_t *enemy, struct player_t *player);

/* observes the current player */
void enemy_observe_current_player(enemy_t *enemy);

/* observes the active player */
void enemy_observe_active_player(enemy_t *enemy);

#endif
