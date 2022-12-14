/*
 * storyboard.c - storyboard (stores the scenes of the game)
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

#include "storyboard.h"
#include "util.h"
#include "../scenes/quest.h"
#include "../scenes/level.h"
#include "../scenes/gameover.h"
#include "../scenes/pause.h"
#include "../scenes/menu.h"
#include "../scenes/intro.h"
#include "../scenes/endofdemo.h"
#include "../scenes/questover.h"
#include "../scenes/confirmbox.h"
#include "../scenes/langselect.h"
#include "../scenes/credits.h"
#include "../scenes/options.h"
#include "../scenes/stageselect.h"

/* private stuff */
#define STORYBOARD_CAPACITY         256     /* up to 256 scenes in the storyboard */
static scene_t *storyboard[STORYBOARD_CAPACITY];
extern scene_t *scene_create(void (*init_func)(), void (*update_func)(), void (*render_func)(), void (*release_func)());
extern void scene_destroy(scene_t *scn);


/*
 * storyboard_init()
 * Initializes the storyboard
 */
void storyboard_init()
{
    int i;

    /* initializing... */
    for(i=0; i<STORYBOARD_CAPACITY; i++)
        storyboard[i] = NULL;

    /* registering the scenes */
    storyboard[SCENE_LEVEL] = scene_create(level_init, level_update, level_render, level_release);
    storyboard[SCENE_PAUSE] = scene_create(pause_init, pause_update, pause_render, pause_release);
    storyboard[SCENE_GAMEOVER] = scene_create(gameover_init, gameover_update, gameover_render, gameover_release);
    storyboard[SCENE_QUEST] = scene_create(quest_init, quest_update, quest_render, quest_release);
    storyboard[SCENE_MENU] = scene_create(menu_init, menu_update, menu_render, menu_release);
    storyboard[SCENE_INTRO] = scene_create(intro_init, intro_update, intro_render, intro_release);
    storyboard[SCENE_ENDOFDEMO] = scene_create(endofdemo_init, endofdemo_update, endofdemo_render, endofdemo_release);
    storyboard[SCENE_QUESTOVER] = scene_create(questover_init, questover_update, questover_render, questover_release);
    storyboard[SCENE_CONFIRMBOX] = scene_create(confirmbox_init, confirmbox_update, confirmbox_render, confirmbox_release);
    storyboard[SCENE_LANGSELECT] = scene_create(langselect_init, langselect_update, langselect_render, langselect_release);
    storyboard[SCENE_CREDITS] = scene_create(credits_init, credits_update, credits_render, credits_release);
    storyboard[SCENE_OPTIONS] = scene_create(options_init, options_update, options_render, options_release);
    storyboard[SCENE_STAGESELECT] = scene_create(stageselect_init, stageselect_update, stageselect_render, stageselect_release);
}




/*
 * storyboard_release()
 * Releases the storyboard
 */
void storyboard_release()
{
    int i;

    for(i=0; i<STORYBOARD_CAPACITY; i++) {
        if(storyboard[i])
            scene_destroy(storyboard[i]);
    }
}




/*
 * storyboard_get_scene()
 * Gets a scene from the storyboard.
 */
scene_t* storyboard_get_scene(scenetype_t type)
{
    int scene_id = clip((int)type, 0, STORYBOARD_CAPACITY-1);
    return storyboard[scene_id];
}

