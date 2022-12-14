/*
 * options.c - options screen
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
#include "options.h"
#include "util/grouptree.h"
#include "../core/scene.h"
#include "../core/storyboard.h"
#include "../core/preferences.h"
#include "../core/util.h"
#include "../core/stringutil.h"
#include "../core/video.h"
#include "../core/audio.h"
#include "../core/lang.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../core/logfile.h"
#include "../core/soundfactory.h"
#include "../entities/font.h"
#include "../entities/actor.h"
#include "../entities/background.h"


/* private data */
#define OPTIONS_BGFILE                  "themes/options.bg"
static int quit, fadein;
static font_t *title;
static actor_t *icon;
static input_t *input;
static scene_t *jump_to;
static float scene_time;
static bgtheme_t *bgtheme;

/* private methods */
static void save_preferences();

/* group tree */
#define OPTIONS_MAX                     8
static int option; /* this is the current option. It lies in the interval [ 0 .. OPTIONS_MAX-1 ] */
static group_t *root;
static group_t *create_grouptree();





/* public functions */

/*
 * options_init()
 * Initializes the scene
 */
void options_init()
{
    option = 0;
    quit = FALSE;
    scene_time = 0;
    input = input_create_user();
    jump_to = NULL;
    fadein = TRUE;

    title = font_create(4);
    font_set_text(title, lang_get("OPTIONS_TITLE"));
    title->position.x = (VIDEO_SCREEN_W - strlen(font_get_text(title))*font_get_charsize(title).x)/2;
    title->position.y = 10;

    bgtheme = background_load(OPTIONS_BGFILE);

    icon = actor_create();
    actor_change_animation(icon, sprite_get_animation("SD_TITLEFOOT", 0));
    icon->position = v2d_new(-50,-50);

    root = create_grouptree();
    grouptree_init_all(root);
}


/*
 * options_release()
 * Releases the scene
 */
void options_release()
{
    grouptree_release_all(root);
    grouptree_destroy_all(root);

    bgtheme = background_unload(bgtheme);

    actor_destroy(icon);
    font_destroy(title);
    input_destroy(input);
}


/*
 * options_update()
 * Updates the scene
 */
void options_update()
{
    float dt = timer_get_delta();
    scene_time += dt;

    /* title */
    font_set_text(title, lang_get("OPTIONS_TITLE"));

    /* fade in */
    if(fadein) {
        fadefx_in(image_rgb(0,0,0), 1.0);
        fadein = FALSE;
    }

    /* background movement */
    background_update(bgtheme);

    /* menu option */
    if(!quit && jump_to == NULL && !fadefx_is_fading()) {
        /* select next option */
        if(input_button_pressed(input, IB_DOWN)) {
            option = (option+1)%OPTIONS_MAX;
            sound_play( soundfactory_get("choose") );
        }

        /* select previous option */
        if(input_button_pressed(input, IB_UP)) {
            option = (((option-1)%OPTIONS_MAX)+OPTIONS_MAX)%OPTIONS_MAX;
            sound_play( soundfactory_get("choose") );
        }

        /* go back... */
        if(input_button_pressed(input, IB_FIRE4)) {
            sound_play( soundfactory_get("return") );
            quit = TRUE;
        }
    }

    /* updating the group tree */
    grouptree_update_all(root);

    /* music */
    if(quit) {
        if(!fadefx_is_fading()) {
            music_stop();
            music_unref(OPTIONS_MUSICFILE);
        }
    }
    else if(!music_is_playing() && scene_time >= 0.2) {
        music_t *m = music_load(OPTIONS_MUSICFILE);
        music_play(m, INFINITY);
    }

    /* quit */
    if(quit) {
        if(fadefx_over()) {
            save_preferences();
            scenestack_pop();
            scenestack_push(storyboard_get_scene(SCENE_MENU));
            return;
        }
        fadefx_out(image_rgb(0,0,0), 1.0);
    }

    /* pushing a scene into the stack */
    if(jump_to != NULL) {
        if(fadefx_over()) {
            save_preferences();
            scenestack_push(jump_to);
            jump_to = NULL;
            fadein = TRUE;
            return;
        }
        fadefx_out(image_rgb(0,0,0), 1.0);
    }
}



/*
 * options_render()
 * Renders the scene
 */
void options_render()
{
    v2d_t cam = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    background_render_bg(bgtheme, cam);
    background_render_fg(bgtheme, cam);

    font_render(title, cam);
    grouptree_render_all(root, cam);
    actor_render(icon, cam);
}






/* private methods */

/* saves the user preferences */
void save_preferences()
{
    preferences_set_videoresolution( video_get_resolution() );
    preferences_set_fullscreen( video_is_fullscreen() );
    preferences_set_smooth( video_is_smooth() );
    preferences_set_showfps( video_is_fps_visible() );
}





/* --------------------------------------- */
/* group tree programming: derived classes */
/* --------------------------------------- */


/* <<abstract>> Fixed label */
void group_fixedlabel_init(group_t *g, char *lang_key)
{
    group_label_init(g);
    g->data = mallocx(256 * sizeof(char));
    str_cpy((char*)(g->data), lang_key, 256);
    font_set_text(g->font, lang_get(lang_key));
}

void group_fixedlabel_release(group_t *g)
{
    free((char*)(g->data));
    group_label_release(g);
}

void group_fixedlabel_update(group_t *g)
{
    group_label_update(g);
    font_set_text(g->font, lang_get((char*)(g->data)));
}

void group_fixedlabel_render(group_t *g, v2d_t camera_position)
{
    group_label_render(g, camera_position);
}


/* <<abstract>> Highlightable label */
typedef struct group_highlightable_data_t {
    int option_index;
    char lang_key[256];
} group_highlightable_data_t;

void group_highlightable_init(group_t *g, char *lang_key, int option_index)
{
    group_highlightable_data_t *data;

    group_label_init(g);
    font_set_text(g->font, lang_get(lang_key));

    g->data = mallocx(sizeof(group_highlightable_data_t));
    data = (group_highlightable_data_t*)(g->data);
    data->option_index = option_index;
    str_cpy(data->lang_key, lang_key, sizeof(data->lang_key));
}

void group_highlightable_release(group_t *g)
{
    free((group_highlightable_data_t*)(g->data));
    group_label_release(g);
}

int group_highlightable_is_highlighted(group_t *g) /* "inheritance" in C */
{
    group_highlightable_data_t *data = (group_highlightable_data_t*)(g->data);
    return (option == data->option_index);
}

void group_highlightable_update(group_t *g)
{
    group_highlightable_data_t *data = (group_highlightable_data_t*)(g->data);

    group_label_update(g);
    font_set_text(g->font, lang_get(data->lang_key));
    if(group_highlightable_is_highlighted(g)) {
        font_set_text(g->font, "<color=ffff00>%s</color>", lang_get(data->lang_key));
        icon->position = v2d_add(g->font->position, v2d_new(-20+3*cos(2*PI*scene_time),0));
    }
}

void group_highlightable_render(group_t *g, v2d_t camera_position)
{
    group_label_render(g, camera_position);
}


/* -------------------------- */


/* Root node */
void group_root_init(group_t *g)
{
    group_label_init(g);
    font_set_text(g->font, "");
    g->font->position = v2d_new(0, 25);
}

void group_root_release(group_t *g)
{
    group_label_release(g);
}

void group_root_update(group_t *g)
{
    group_label_update(g);
}

void group_root_render(group_t *g, v2d_t camera_position)
{
    group_label_render(g, camera_position);
}

group_t* group_root_create()
{
    return group_create(group_root_init, group_root_release, group_root_update, group_root_render);
}


/* "Graphics" label */
void group_graphics_init(group_t *g)
{
    group_fixedlabel_init(g, "OPTIONS_GRAPHICS");
}

void group_graphics_release(group_t *g)
{
    group_fixedlabel_release(g);
}

void group_graphics_update(group_t *g)
{
    group_fixedlabel_update(g);
}

void group_graphics_render(group_t *g, v2d_t camera_position)
{
    group_fixedlabel_render(g, camera_position);
}

group_t *group_graphics_create()
{
    return group_create(group_graphics_init, group_graphics_release, group_graphics_update, group_graphics_render);
}


/* "Fullscreen" label */
void group_fullscreen_init(group_t *g)
{
    group_highlightable_init(g, "OPTIONS_FULLSCREEN", 1);
}

void group_fullscreen_release(group_t *g)
{
    group_highlightable_release(g);
}

int group_fullscreen_is_highlighted(group_t *g)
{
    return group_highlightable_is_highlighted(g);
}

void group_fullscreen_update(group_t *g)
{
    /* base class */
    group_highlightable_update(g);

    /* derived class */
    if(group_fullscreen_is_highlighted(g)) {
        if(!fadefx_is_fading()) {
            if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                sound_play( soundfactory_get("select") );
                video_changemode(video_get_resolution(), video_is_smooth(), !video_is_fullscreen());
            }
            if(input_button_pressed(input, IB_RIGHT)) {
                if(video_is_fullscreen()) {
                    sound_play( soundfactory_get("select") );
                    video_changemode(video_get_resolution(), video_is_smooth(), FALSE);
                }
            }
            if(input_button_pressed(input, IB_LEFT)) {
                if(!video_is_fullscreen()) {
                    sound_play( soundfactory_get("select") );
                    video_changemode(video_get_resolution(), video_is_smooth(), TRUE);
                }
            }
        }
    }
}

void group_fullscreen_render(group_t *g, v2d_t camera_position)
{
    font_t *f;
    char v[2][80];

    /* base class */
    group_highlightable_render(g, camera_position);

    /* derived class */
    f = font_create(8);
    f->position = v2d_new(175, g->font->position.y);

    str_cpy(v[0], lang_get("OPTIONS_YES"), sizeof(v[0]));
    str_cpy(v[1], lang_get("OPTIONS_NO"), sizeof(v[1]));

    if(video_is_fullscreen())
        font_set_text(f, "<color=ffff00>%s</color>  %s", v[0], v[1]);
    else
        font_set_text(f, "%s  <color=ffff00>%s</color>", v[0], v[1]);

    font_render(f, camera_position);
    font_destroy(f);
}

group_t *group_fullscreen_create()
{
    return group_create(group_fullscreen_init, group_fullscreen_release, group_fullscreen_update, group_fullscreen_render);
}


/* "Smooth Graphics" label */
void group_smooth_init(group_t *g)
{
    group_highlightable_init(g, "OPTIONS_SMOOTHGFX", 2);
}

void group_smooth_release(group_t *g)
{
    group_highlightable_release(g);
}

int group_smooth_is_highlighted(group_t *g)
{
    return group_highlightable_is_highlighted(g);
}

void group_smooth_update(group_t *g)
{
    int resolution = (video_get_resolution() == VIDEORESOLUTION_1X) ? VIDEORESOLUTION_2X : video_get_resolution();

    /* base class */
    group_highlightable_update(g);

    /* derived class */
    if(group_smooth_is_highlighted(g)) {
        if(!fadefx_is_fading()) {
            if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                sound_play( soundfactory_get("select") );
                video_changemode(resolution, !video_is_smooth(), video_is_fullscreen());
            }
            if(input_button_pressed(input, IB_RIGHT)) {
                if(video_is_smooth()) {
                    sound_play( soundfactory_get("select") );
                    video_changemode(resolution, FALSE, video_is_fullscreen());
                }
            }
            if(input_button_pressed(input, IB_LEFT)) {
                if(!video_is_smooth()) {
                    sound_play( soundfactory_get("select") );
                    video_changemode(resolution, TRUE, video_is_fullscreen());
                }
            }
        }
    }
}

void group_smooth_render(group_t *g, v2d_t camera_position)
{
    font_t *f;
    char v[2][80];

    /* base class */
    group_highlightable_render(g, camera_position);

    /* derived class */
    f = font_create(8);
    f->position = v2d_new(175, g->font->position.y);

    str_cpy(v[0], lang_get("OPTIONS_YES"), sizeof(v[0]));
    str_cpy(v[1], lang_get("OPTIONS_NO"), sizeof(v[1]));

    if(video_is_smooth())
        font_set_text(f, "<color=ffff00>%s</color>  %s", v[0], v[1]);
    else
        font_set_text(f, "%s  <color=ffff00>%s</color>", v[0], v[1]);

    font_render(f, camera_position);
    font_destroy(f);
}

group_t *group_smooth_create()
{
    return group_create(group_smooth_init, group_smooth_release, group_smooth_update, group_smooth_render);
}


/* "Show FPS" label */
void group_fps_init(group_t *g)
{
    group_highlightable_init(g, "OPTIONS_FPS", 3);
}

void group_fps_release(group_t *g)
{
    group_highlightable_release(g);
}

int group_fps_is_highlighted(group_t *g)
{
    return group_highlightable_is_highlighted(g);
}

void group_fps_update(group_t *g)
{
    /* base class */
    group_highlightable_update(g);

    /* derived class */
    if(group_fps_is_highlighted(g)) {
        if(!fadefx_is_fading()) {
            if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                sound_play( soundfactory_get("select") );
                video_show_fps(!video_is_fps_visible());
            }
            if(input_button_pressed(input, IB_RIGHT)) {
                if(video_is_fps_visible()) {
                    sound_play( soundfactory_get("select") );
                    video_show_fps(FALSE);
                }
            }
            if(input_button_pressed(input, IB_LEFT)) {
                if(!video_is_fps_visible()) {
                    sound_play( soundfactory_get("select") );
                    video_show_fps(TRUE);
                }
            }
        }
    }
}

void group_fps_render(group_t *g, v2d_t camera_position)
{
    font_t *f;
    char v[2][80];

    /* base class */
    group_highlightable_render(g, camera_position);

    /* derived class */
    f = font_create(8);
    f->position = v2d_new(175, g->font->position.y);

    str_cpy(v[0], lang_get("OPTIONS_YES"), sizeof(v[0]));
    str_cpy(v[1], lang_get("OPTIONS_NO"), sizeof(v[1]));

    if(video_is_fps_visible())
        font_set_text(f, "<color=ffff00>%s</color>  %s", v[0], v[1]);
    else
        font_set_text(f, "%s  <color=ffff00>%s</color>", v[0], v[1]);

    font_render(f, camera_position);
    font_destroy(f);
}

group_t *group_fps_create()
{
    return group_create(group_fps_init, group_fps_release, group_fps_update, group_fps_render);
}


/* "Resolution" label */
void group_resolution_init(group_t *g)
{
    group_highlightable_init(g, "OPTIONS_RESOLUTION", 0);
}

void group_resolution_release(group_t *g)
{
    group_highlightable_release(g);
}

int group_resolution_is_highlighted(group_t *g)
{
    return group_highlightable_is_highlighted(g);
}

void group_resolution_update(group_t *g)
{
    /* base class */
    group_highlightable_update(g);

    /* derived class */
    if(group_resolution_is_highlighted(g)) {
        if(!fadefx_is_fading()) {
            if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                switch(video_get_resolution()) {
                    case VIDEORESOLUTION_1X:
                        video_changemode(VIDEORESOLUTION_2X, video_is_smooth(), video_is_fullscreen());
                        sound_play( soundfactory_get("select") );
                        break;
                    case VIDEORESOLUTION_2X:
                        video_changemode(VIDEORESOLUTION_MAX, video_is_smooth(), video_is_fullscreen());
                        sound_play( soundfactory_get("select") );
                        break;
                    case VIDEORESOLUTION_MAX:
                        video_changemode(VIDEORESOLUTION_1X, video_is_smooth(), video_is_fullscreen());
                        sound_play( soundfactory_get("select") );
                        break;
                }
            }
            if(input_button_pressed(input, IB_RIGHT)) {
                switch(video_get_resolution()) {
                    case VIDEORESOLUTION_1X:
                        video_changemode(VIDEORESOLUTION_2X, video_is_smooth(), video_is_fullscreen());
                        sound_play( soundfactory_get("select") );
                        break;
                    case VIDEORESOLUTION_2X:
                        video_changemode(VIDEORESOLUTION_MAX, video_is_smooth(), video_is_fullscreen());
                        sound_play( soundfactory_get("select") );
                        break;
                }
            }
            if(input_button_pressed(input, IB_LEFT)) {
                switch(video_get_resolution()) {
                    case VIDEORESOLUTION_MAX:
                        video_changemode(VIDEORESOLUTION_2X, video_is_smooth(), video_is_fullscreen());
                        sound_play( soundfactory_get("select") );
                        break;
                    case VIDEORESOLUTION_2X:
                        video_changemode(VIDEORESOLUTION_1X, video_is_smooth(), video_is_fullscreen());
                        sound_play( soundfactory_get("select") );
                        break;
                }
            }
        }
    }
}

void group_resolution_render(group_t *g, v2d_t camera_position)
{
    font_t *f;
    char v[3][80];

    /* base class */
    group_highlightable_render(g, camera_position);

    /* derived class */
    f = font_create(8);
    f->position = v2d_new(175, g->font->position.y);

    str_cpy(v[0], lang_get("OPTIONS_RESOLUTION_OPT1"), sizeof(v[0]));
    str_cpy(v[1], lang_get("OPTIONS_RESOLUTION_OPT2"), sizeof(v[1]));
    str_cpy(v[2], lang_get("OPTIONS_RESOLUTION_OPT3"), sizeof(v[2]));

    switch(video_get_resolution()) {
        case VIDEORESOLUTION_1X:
            font_set_text(f, "<color=ffff00>%s</color> %s %s", v[0], v[1], v[2]);
            break;

        case VIDEORESOLUTION_2X:
            font_set_text(f, "%s <color=ffff00>%s</color> %s", v[0], v[1], v[2]);
            break;

        case VIDEORESOLUTION_MAX:
            font_set_text(f, "%s %s <color=ffff00>%s</color>", v[0], v[1], v[2]);
            break;
    }

    font_render(f, camera_position);
    font_destroy(f);
}

group_t *group_resolution_create()
{
    return group_create(group_resolution_init, group_resolution_release, group_resolution_update, group_resolution_render);
}

/* "Game" label */
void group_game_init(group_t *g)
{
    group_fixedlabel_init(g, "OPTIONS_GAME");
}

void group_game_release(group_t *g)
{
    group_fixedlabel_release(g);
}

void group_game_update(group_t *g)
{
    group_fixedlabel_update(g);
}

void group_game_render(group_t *g, v2d_t camera_position)
{
    group_fixedlabel_render(g, camera_position);
}

group_t *group_game_create()
{
    return group_create(group_game_init, group_game_release, group_game_update, group_game_render);
}

/* "Change Language" label */
void group_changelanguage_init(group_t *g)
{
    group_highlightable_init(g, "OPTIONS_LANGUAGE", 4);
}

void group_changelanguage_release(group_t *g)
{
    group_highlightable_release(g);
}

int group_changelanguage_is_highlighted(group_t *g)
{
    return group_highlightable_is_highlighted(g);
}

void group_changelanguage_update(group_t *g)
{
    /* base class */
    group_highlightable_update(g);

    /* derived class */
    if(group_changelanguage_is_highlighted(g)) {
        if(!fadefx_is_fading()) {
            if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                sound_play( soundfactory_get("select") );
                jump_to = storyboard_get_scene(SCENE_LANGSELECT);
            }
        }
    }
}

void group_changelanguage_render(group_t *g, v2d_t camera_position)
{
    group_highlightable_render(g, camera_position);
}

group_t *group_changelanguage_create()
{
    return group_create(group_changelanguage_init, group_changelanguage_release, group_changelanguage_update, group_changelanguage_render);
}

/* "Credits" label */
void group_credits_init(group_t *g)
{
    group_highlightable_init(g, "OPTIONS_CREDITS", 6);
}

void group_credits_release(group_t *g)
{
    group_highlightable_release(g);
}

int group_credits_is_highlighted(group_t *g)
{
    return group_highlightable_is_highlighted(g);
}

void group_credits_update(group_t *g)
{
    /* base class */
    group_highlightable_update(g);

    /* derived class */
    if(group_credits_is_highlighted(g)) {
        if(!fadefx_is_fading()) {
            if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                sound_play( soundfactory_get("select") );
                jump_to = storyboard_get_scene(SCENE_CREDITS);
            }
        }
    }
}

void group_credits_render(group_t *g, v2d_t camera_position)
{
    group_highlightable_render(g, camera_position);
}

group_t *group_credits_create()
{
    return group_create(group_credits_init, group_credits_release, group_credits_update, group_credits_render);
}

/* "Stage Select" label */
void group_stageselect_init(group_t *g)
{
    group_highlightable_init(g, "OPTIONS_STAGESELECT", 5);
}

void group_stageselect_release(group_t *g)
{
    group_highlightable_release(g);
}

int group_stageselect_is_highlighted(group_t *g)
{
    return group_highlightable_is_highlighted(g);
}

void group_stageselect_update(group_t *g)
{
    /* base class */
    group_highlightable_update(g);

    /* derived class */
    if(group_stageselect_is_highlighted(g)) {
        if(!fadefx_is_fading()) {
            if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                sound_play( soundfactory_get("select") );
                jump_to = storyboard_get_scene(SCENE_STAGESELECT);
            }
        }
    }
}

void group_stageselect_render(group_t *g, v2d_t camera_position)
{
    group_highlightable_render(g, camera_position);
}

group_t *group_stageselect_create()
{
    return group_create(group_stageselect_init, group_stageselect_release, group_stageselect_update, group_stageselect_render);
}

/* "Back" label */
void group_back_init(group_t *g)
{
    group_highlightable_init(g, "OPTIONS_BACK", 7);
}

void group_back_release(group_t *g)
{
    group_highlightable_release(g);
}

int group_back_is_highlighted(group_t *g)
{
    return group_highlightable_is_highlighted(g);
}

void group_back_update(group_t *g)
{
    /* base class */
    group_highlightable_update(g);

    /* derived class */
    if(group_back_is_highlighted(g)) {
        if(!fadefx_is_fading()) {
            if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                sound_play( soundfactory_get("select") );
                quit = TRUE;
            }
        }
    }
}

void group_back_render(group_t *g, v2d_t camera_position)
{
    group_highlightable_render(g, camera_position);
}

group_t *group_back_create()
{
    return group_create(group_back_init, group_back_release, group_back_update, group_back_render);
}




/* ----------------------------------------- */
/* group tree programming: creating the tree */
/* ----------------------------------------- */

/* creates the group tree */
group_t *create_grouptree()
{
    group_t *root;
    group_t *graphics, *fullscreen, *resolution, *smooth, *fps;
    group_t *game, *changelanguage, *credits, *stageselect;
    group_t *back;

    /* section: graphics */
    resolution = group_resolution_create();
    fullscreen = group_fullscreen_create();
    smooth = group_smooth_create();
    fps = group_fps_create();
    graphics = group_graphics_create();
    group_addchild(graphics, resolution);
    group_addchild(graphics, fullscreen);
    group_addchild(graphics, smooth);
    group_addchild(graphics, fps);

    /* section: game */
    changelanguage = group_changelanguage_create();
    credits = group_credits_create();
    stageselect = group_stageselect_create();
    game = group_game_create();
    group_addchild(game, changelanguage);
    group_addchild(game, stageselect);
    group_addchild(game, credits);

    /* back */
    back = group_back_create();

    /* section: root */
    root = group_root_create();
    group_addchild(root, graphics);
    group_addchild(root, game);
    group_addchild(root, back);

    /* done! */
    return root;
}

