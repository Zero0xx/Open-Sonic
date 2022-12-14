/*
 * confirmbox.c - confirm box
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

#include <math.h>
#include "confirmbox.h"
#include "../entities/actor.h"
#include "../entities/font.h"
#include "../core/video.h"
#include "../core/sprite.h"
#include "../core/input.h"
#include "../core/audio.h"
#include "../core/timer.h"
#include "../core/scene.h"
#include "../core/soundfactory.h"


/* private data */
#define MAX_OPTIONS 5
#define NO_OPTION   -1
static image_t *box, *background;
static v2d_t boxpos;
static font_t *textfnt;
static font_t *optionfnt[MAX_OPTIONS][2];
static actor_t *icon;
static input_t *input;
static char text[1024], option[MAX_OPTIONS][128];
static int option_count;
static int current_option = NO_OPTION;
static int fxfade_in, fxfade_out;


/* public functions */

/*
 * confirmbox_init()
 * Initializes this scene. Please remember to
 * call confirmbox_alert() before starting this
 * scene!
 */
void confirmbox_init()
{
    int i;

    background = image_create(video_get_backbuffer()->w, video_get_backbuffer()->h);
    image_blit(video_get_backbuffer(), background, 0, 0, 0, 0, video_get_backbuffer()->w, video_get_backbuffer()->h);

    box = sprite_get_image(sprite_get_animation("SD_CONFIRMBOX", 0), 0);
    boxpos = v2d_new( (VIDEO_SCREEN_W-box->w)/2 , VIDEO_SCREEN_H );

    input = input_create_user();
    icon = actor_create();
    actor_change_animation(icon, sprite_get_animation("SD_TITLEFOOT", 0));

    textfnt = font_create(8);
    font_set_text(textfnt, text);
    font_set_width(textfnt, 164);

    for(i=0; i<option_count; i++) {
        optionfnt[i][0] = font_create(8);
        optionfnt[i][1] = font_create(8);
        font_set_text(optionfnt[i][0], option[i]);
        font_set_text(optionfnt[i][1], "<color=ffff00>%s</color>", option[i]);
    }

    current_option = 0;
    fxfade_in = TRUE;
    fxfade_out = FALSE;
}


/*
 * confirmbox_release()
 * Releases the scene
 */
void confirmbox_release()
{
    int i;

    for(i=0; i<option_count; i++) {
        font_destroy(optionfnt[i][0]);
        font_destroy(optionfnt[i][1]);
    }

    actor_destroy(icon);
    input_destroy(input);
    font_destroy(textfnt);
    image_destroy(background);
}


/*
 * confirmbox_update()
 * Updates the scene
 */
void confirmbox_update()
{
    int i;
    float dt = timer_get_delta(), speed = 5*VIDEO_SCREEN_H;

    /* fade-in */
    if(fxfade_in) {
        if( boxpos.y <= (VIDEO_SCREEN_H-box->h)/2 )
            fxfade_in = FALSE;
        else
            boxpos.y -= speed*dt;
    }

    /* fade-out */
    if(fxfade_out) {
        if( boxpos.y >= VIDEO_SCREEN_H ) {
            fxfade_out = FALSE;
            scenestack_pop();
            return;
        }
        else
            boxpos.y += speed*dt;
    }

    /* positioning stuff */
    icon->position = v2d_new(boxpos.x + current_option*box->w/option_count + 10, boxpos.y + box->h*0.75 - 1);
    textfnt->position = v2d_new(boxpos.x + 10 , boxpos.y + 10);
    for(i=0; i<option_count; i++) {
        optionfnt[i][0]->position = v2d_new(boxpos.x + i*box->w/option_count + 25, boxpos.y + box->h*0.75);
        optionfnt[i][1]->position = optionfnt[i][0]->position;
    }

    /* input */
    if(!fxfade_in && !fxfade_out) {
        if(input_button_pressed(input, IB_LEFT)) {
            /* left */
            sound_play( soundfactory_get("choose") );
            current_option = ( ((current_option-1)%option_count) + option_count )%option_count;
        }
        else if(input_button_pressed(input, IB_RIGHT)) {
            /* right */
            sound_play( soundfactory_get("choose") );
            current_option = (current_option+1)%option_count;
        }
        else if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
            /* confirm */
            sound_play( soundfactory_get("select") );
            fxfade_out = TRUE;
        }
    }
}



/*
 * confirmbox_render()
 * Renders the scene
 */
void confirmbox_render()
{
    int i, k;
    v2d_t cam = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    image_blit(background, video_get_backbuffer(), 0, 0, 0, 0, background->w, background->h);
    image_draw(box, video_get_backbuffer(), boxpos.x, boxpos.y, IF_NONE);
    font_render(textfnt, cam);

    for(i=0; i<option_count; i++) {
        k = (i==current_option) ? 1 : 0;
        font_render(optionfnt[i][k], cam);
    }

    actor_render(icon, cam);
}




/*
 * confirmbox_alert()
 * Configures this scene (call me before initializing this scene!)
 * PS: option2 may be NULL
 */
void confirmbox_alert(char *ptext, char *option1, char *option2)
{
    current_option = -1;
    strcpy(text, ptext);
    strcpy(option[0], option1);

    if(option2) {
        strcpy(option[1], option2);
        option_count = 2;
    }
    else
        option_count = 1;
}


/*
 * confirmbox_selected_option()
 * Returns the selected option (1, 2, ..., n), or
 * 0 if nothing has been selected.
 * This must be called AFTER this scene
 * gets released
 */
int confirmbox_selected_option()
{
    if(current_option != NO_OPTION) {
        int ret = current_option + 1;
        current_option = NO_OPTION;
        return ret;
    }
    else
        return 0; /* nothing */
}

