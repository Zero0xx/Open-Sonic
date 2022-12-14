/*
 * itembox.c - item boxes
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

#include "itembox.h"
#include "icon.h"
#include "../../scenes/level.h"
#include "../../core/audio.h"
#include "../../core/util.h"
#include "../../core/soundfactory.h"

/* itembox class */
typedef struct itembox_t itembox_t;
struct itembox_t {
    item_t item; /* base class */
    int anim_id; /* animation id */
    void (*on_destroy)(item_t*,player_t*); /* strategy pattern */
};

static item_t* itembox_create(void (*on_destroy)(item_t*,player_t*), int anim_id);
static void itembox_init(item_t *item);
static void itembox_release(item_t* item);
static void itembox_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void itembox_render(item_t* item, v2d_t camera_position);
static int itembox_player_collision(item_t *box, player_t *player);

static void lifebox_strategy(item_t *item, player_t *player);
static void ringbox_strategy(item_t *item, player_t *player);
static void starbox_strategy(item_t *item, player_t *player);
static void speedbox_strategy(item_t *item, player_t *player);
static void glassesbox_strategy(item_t *item, player_t *player);
static void shieldbox_strategy(item_t *item, player_t *player);
static void fireshieldbox_strategy(item_t *item, player_t *player);
static void thundershieldbox_strategy(item_t *item, player_t *player);
static void watershieldbox_strategy(item_t *item, player_t *player);
static void acidshieldbox_strategy(item_t *item, player_t *player);
static void windshieldbox_strategy(item_t *item, player_t *player);
static void trapbox_strategy(item_t *item, player_t *player);
static void emptybox_strategy(item_t *item, player_t *player);



/* public methods */
item_t* lifebox_create()
{
    return itembox_create(lifebox_strategy, 0); /* (strategy, animation id) */
}

item_t* ringbox_create()
{
    return itembox_create(ringbox_strategy, 3);
}

item_t* starbox_create()
{
    return itembox_create(starbox_strategy, 4);
}

item_t* speedbox_create()
{ 
   return itembox_create(speedbox_strategy, 5);
}

item_t* glassesbox_create()
{
    return itembox_create(glassesbox_strategy, 6);
}

item_t* shieldbox_create()
{
    return itembox_create(shieldbox_strategy, 7);
}

item_t* trapbox_create()
{
    return itembox_create(trapbox_strategy, 8);
}

item_t* emptybox_create()
{
    return itembox_create(emptybox_strategy, 9);
}

item_t* fireshieldbox_create()
{
    return itembox_create(fireshieldbox_strategy, 11);
}

item_t* thundershieldbox_create()
{
    return itembox_create(thundershieldbox_strategy, 12);
}

item_t* watershieldbox_create()
{
    return itembox_create(watershieldbox_strategy, 13);
}

item_t* acidshieldbox_create()
{
    return itembox_create(acidshieldbox_strategy, 14);
}

item_t* windshieldbox_create()
{
    return itembox_create(windshieldbox_strategy, 15);
}


/* private strategies */
void lifebox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player_set_lives( player_get_lives()+1 );
    level_override_music( soundfactory_get("1up") );
}

void ringbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player_set_rings( player_get_rings()+10 );
    sound_play( soundfactory_get("ring") );
}

void starbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->invincible = TRUE;
    player->invtimer = 0;
    music_play( music_load("musics/invincible.ogg"), 0 );
}

void speedbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->got_speedshoes = TRUE;
    player->speedshoes_timer = 0;
    music_play( music_load("musics/speed.ogg"), 0 );
}

void glassesbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->got_glasses = TRUE;
}

void shieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_SHIELD;
    sound_play( soundfactory_get("shield") );
}

void fireshieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_FIRESHIELD;
    sound_play( soundfactory_get("fire shield") );
}

void thundershieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_THUNDERSHIELD;
    sound_play( soundfactory_get("thunder shield") );
}

void watershieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_WATERSHIELD;
    sound_play( soundfactory_get("water shield") );
}

void acidshieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_ACIDSHIELD;
    sound_play( soundfactory_get("acid shield") );
}

void windshieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_WINDSHIELD;
    sound_play( soundfactory_get("wind shield") );
}

void trapbox_strategy(item_t *item, player_t *player)
{
    player_hit(player);
}

void emptybox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
}


/* private methods */
item_t* itembox_create(void (*on_destroy)(item_t*,player_t*), int anim_id)
{
    item_t *item = mallocx(sizeof(itembox_t));
    itembox_t *me = (itembox_t*)item;

    item->init = itembox_init;
    item->release = itembox_release;
    item->update = itembox_update;
    item->render = itembox_render;

    me->on_destroy = on_destroy;
    me->anim_id = anim_id;

    return item;
}

void itembox_init(item_t *item)
{
    itembox_t *me = (itembox_t*)item;

    item->obstacle = TRUE;
    item->bring_to_back = FALSE;
    item->preserve = TRUE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_ITEMBOX", me->anim_id));
}

void itembox_release(item_t* item)
{
    actor_destroy(item->actor);
}

void itembox_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;
    actor_t *act = item->actor;
    itembox_t *me = (itembox_t*)item;

    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!(player->actor->is_jumping && player->actor->speed.y < -10)) {

            /* the player is about to crash this box... */
            if(item->state == IS_IDLE && itembox_player_collision(item, player) && player_attacking(player)) {
                item_t *icon = level_create_item(IT_ICON, v2d_add(act->position, v2d_new(0,-5)));
                icon_change_animation(icon, me->anim_id);
                level_create_item(IT_EXPLOSION, v2d_add(act->position, v2d_new(0,-20)));
                level_create_item(IT_CRUSHEDBOX, act->position);

                sound_play( soundfactory_get("destroy") );
                if(player->actor->is_jumping)
                    player_bounce(player);

                me->on_destroy(item, player);
                item->state = IS_DEAD;
            }

        }
    }

    /* animation */
    me->anim_id = me->anim_id < 3 ? level_player_id() : me->anim_id;
    actor_change_animation(item->actor, sprite_get_animation("SD_ITEMBOX", me->anim_id));
}

void itembox_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}

/* returns TRUE if player collides with box, or
 * FALSE otherwise. As fake bricks are created
 * on top of boxes, a simple actor_collision()
 * doesn't do the job. */
int itembox_player_collision(item_t *box, player_t *player)
{
    /* fake bricks are created on top of
     * the item boxes. Therefore, boxes need to
     * be adjusted in order to handle collisions
     * with the player properly */
    int collided;
    actor_t *act = box->actor;
    v2d_t oldpos = act->position;

    /* hack */
    act->position.y -= 5; /* jump fix */
    if(player->spin) { /* spindash through multiple boxes */
        if(player->actor->position.x < act->position.x && player->actor->speed.x > 0)
            act->position.x -= 15;
        else if(player->actor->position.x > act->position.x && player->actor->speed.x < 0)
            act->position.x += 15;
    }

    /* collision detection */
    collided = actor_collision(box->actor, player->actor);

    /* unhack */
    box->actor->position = oldpos;

    /* done! */
    return collided;
}
