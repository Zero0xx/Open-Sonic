/*
 * pause.c - "pause" scene
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

#include <math.h>
#include "pause.h"
#include "confirmbox.h"
#include "quest.h"
#include "../core/scene.h"
#include "../core/storyboard.h"
#include "../core/util.h"
#include "../core/video.h"
#include "../core/input.h"
#include "../core/lang.h"
#include "../core/sprite.h"
#include "../core/audio.h"
#include "../core/timer.h"


/* private data */
static image_t *pause_buf;
static input_t *pause_input;
static int pause_ready, pause_quit;
static float pause_timer;


/*
 * pause_init()
 * Initializes the pause screen
 */
void pause_init()
{
    pause_input = input_create_user();
    pause_buf = image_create(video_get_backbuffer()->w, video_get_backbuffer()->h);
    pause_ready = FALSE;
    pause_quit = FALSE;
    pause_timer = 0;
    image_blit(video_get_backbuffer(), pause_buf, 0, 0, 0, 0, pause_buf->w, pause_buf->h);
}


/*
 * pause_release()
 * Releases the pause screen
 */
void pause_release()
{
    image_destroy(pause_buf);
    input_destroy(pause_input);
}


/*
 * pause_update()
 * Updates the pause screen
 */
void pause_update()
{
    /* quit */
    if(input_button_pressed(pause_input, IB_FIRE4)) {
        char op[3][512];

        lang_getstring("CBOX_QUIT_QUESTION", op[0], sizeof(op[0]));
        lang_getstring("CBOX_QUIT_OPTION1", op[1], sizeof(op[1]));
        lang_getstring("CBOX_QUIT_OPTION2", op[2], sizeof(op[2]));
        confirmbox_alert(op[0], op[1], op[2]);

        scenestack_push(storyboard_get_scene(SCENE_CONFIRMBOX));
        return;
    }

    if(1 == confirmbox_selected_option())
        pause_quit = TRUE;

    if(pause_quit) {
        if(fadefx_over()) {
            scenestack_pop();
            scenestack_pop();
            quest_abort();
            return;
        }
        fadefx_out(image_rgb(0,0,0), 1.0);
        return;
    }

    /* unpause */
    if(pause_ready) {
        if(input_button_pressed(pause_input, IB_FIRE3)) {
            music_resume();
            scenestack_pop();
            return;
        }
    }
    else {
        if(input_button_up(pause_input, IB_FIRE3))
            pause_ready = TRUE;
    }
}


/* 
 * pause_render()
 * Renders the pause screen
 */
void pause_render()
{
    image_t *p = sprite_get_image(sprite_get_animation("SD_PAUSE", 0), 0);
    float scale = 1+0.5*fabs(cos(PI/2*pause_timer));
    v2d_t pos = v2d_new((VIDEO_SCREEN_W-p->w)/2 - (scale-1)*p->w/2, (VIDEO_SCREEN_H-p->h)/2 - (scale-1)*p->h/2);

    image_blit(pause_buf, video_get_backbuffer(), 0, 0, 0, 0, pause_buf->w, pause_buf->h);
    image_draw_scaled(p, video_get_backbuffer(), (int)pos.x, (int)pos.y, v2d_new(scale,scale), IF_NONE);

    if(!pause_quit)
        pause_timer += timer_get_delta();
}
