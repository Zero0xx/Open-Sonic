/*
 * stageselect.c - stage selection screen
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
#include <ctype.h>
#include "stageselect.h"
#include "options.h"
#include "level.h"
#include "../core/util.h"
#include "../core/scene.h"
#include "../core/storyboard.h"
#include "../core/v2d.h"
#include "../core/osspec.h"
#include "../core/stringutil.h"
#include "../core/logfile.h"
#include "../core/video.h"
#include "../core/audio.h"
#include "../core/lang.h"
#include "../core/input.h"
#include "../core/timer.h"
#include "../core/soundfactory.h"
#include "../core/nanoparser/nanoparser.h"
#include "../entities/font.h"
#include "../entities/actor.h"
#include "../entities/background.h"



/* stage data */
typedef struct {
    char filepath[1024]; /* absolute filepath */
    char name[128]; /* stage name */
    int act; /* act number */
    int requires[3]; /* required version */
    char bgtheme[1024]; /* background theme */
} stagedata_t;

static stagedata_t* stagedata_load(const char *filename);
static void stagedata_unload(stagedata_t *s);
static int traverse(const parsetree_statement_t *stmt, void *stagedata);



/* private data */
#define STAGE_BGFILE             "themes/levelselect.bg"
#define STAGE_MAXPERPAGE         8
static font_t *title; /* title */
static font_t *msg; /* message */
static font_t *page; /* page number */
static actor_t *icon; /* cursor icon */
static input_t *input; /* input device */
static float scene_time; /* scene time, in seconds */
static bgtheme_t *bgtheme; /* background */

static enum { STAGESTATE_NORMAL, STAGESTATE_QUIT, STAGESTATE_PLAY, STAGESTATE_FADEIN } state; /* finite state machine */

static stagedata_t **stage_data; /* vector of stagedata_t* */
static int stage_count; /* length of stage_data[] */
static int option; /* current option: 0 .. stage_count - 1 */
static font_t **stage_label; /* vector */



/* private functions */
static void load_stage_list();
static void unload_stage_list();
static int dirfill(const char *filename, int attrib, void *param);
static int dircount(const char *filename, int attrib, void *param);
static int sort_cmp(const void *a, const void *b);






/* public functions */

/*
 * stageselect_init()
 * Initializes the scene
 */
void stageselect_init()
{
    option = 0;
    scene_time = 0;
    state = STAGESTATE_NORMAL;
    input = input_create_user();

    title = font_create(4);
    font_set_text(title, lang_get("STAGESELECT_TITLE"));
    title->position.x = (VIDEO_SCREEN_W - strlen(font_get_text(title))*font_get_charsize(title).x)/2;
    title->position.y = 10;

    msg = font_create(8);
    font_set_text(msg, lang_get("STAGESELECT_MSG"));
    msg->position.x = 10;
    msg->position.y = VIDEO_SCREEN_H-font_get_charsize(msg).y*1.5;

    page = font_create(8);
    font_set_text(page, lang_get("STAGESELECT_PAGE"), 0, 0);
    page->position.x = VIDEO_SCREEN_W - strlen(font_get_text(page))*font_get_charsize(page).x - 10;
    page->position.y = 40;

    bgtheme = background_load(STAGE_BGFILE);

    icon = actor_create();
    actor_change_animation(icon, sprite_get_animation("SD_TITLEFOOT", 0));

    load_stage_list();
    fadefx_in(image_rgb(0,0,0), 1.0);
}


/*
 * stageselect_release()
 * Releases the scene
 */
void stageselect_release()
{
    unload_stage_list();
    bgtheme = background_unload(bgtheme);

    actor_destroy(icon);
    font_destroy(title);
    font_destroy(msg);
    font_destroy(page);
    input_destroy(input);
}


/*
 * stageselect_update()
 * Updates the scene
 */
void stageselect_update()
{
    int pagenum, maxpages;
    float dt = timer_get_delta();
    scene_time += dt;

    /* background movement */
    background_update(bgtheme);

    /* menu option */
    icon->position = stage_label[option]->position;
    icon->position.x += -20 + 3*cos(2*PI * scene_time);

    /* page number */
    pagenum = option/STAGE_MAXPERPAGE + 1;
    maxpages = stage_count/STAGE_MAXPERPAGE + ((stage_count%STAGE_MAXPERPAGE == 0) ? 0 : 1);
    font_set_text(page, lang_get("STAGESELECT_PAGE"), pagenum, maxpages);
    page->position.x = VIDEO_SCREEN_W - strlen(font_get_text(page))*font_get_charsize(page).x - 10;

    /* music */
    if(state == STAGESTATE_PLAY) {
        if(!fadefx_is_fading()) {
            music_stop();
            music_unref(OPTIONS_MUSICFILE);
        }
    }
    else if(!music_is_playing()) {
        music_t *m = music_load(OPTIONS_MUSICFILE);
        music_play(m, INFINITY);
    }

    /* finite state machine */
    switch(state) {
        /* normal mode (menu) */
        case STAGESTATE_NORMAL: {
            if(!fadefx_is_fading()) {
                if(input_button_pressed(input, IB_DOWN)) {
                    option = (option+1) % stage_count;
                    sound_play( soundfactory_get("choose") );
                }
                if(input_button_pressed(input, IB_UP)) {
                    option = (((option-1) % stage_count) + stage_count) % stage_count;
                    sound_play( soundfactory_get("choose") );
                }
                if(input_button_pressed(input, IB_FIRE1) || input_button_pressed(input, IB_FIRE3)) {
                    logfile_message("Loading level \"%s\", \"%s\"", stage_data[option]->name, stage_data[option]->filepath);
                    level_setfile(stage_data[option]->filepath);
                    sound_play( soundfactory_get("select") );
                    state = STAGESTATE_PLAY;
                }
                if(input_button_pressed(input, IB_FIRE4)) {
                    sound_play( soundfactory_get("return") );
                    state = STAGESTATE_QUIT;
                }
            }
            break;
        }

        /* fade-out effect (quit this screen) */
        case STAGESTATE_QUIT: {
            if(fadefx_over()) {
                scenestack_pop();
                return;
            }
            fadefx_out(image_rgb(0,0,0), 1.0);
            break;
        }

        /* fade-out effect (play a level) */
        case STAGESTATE_PLAY: {
            if(fadefx_over()) {
                player_set_lives(PLAYER_INITIAL_LIVES);
                player_set_score(0);
                scenestack_push(storyboard_get_scene(SCENE_LEVEL));
                state = STAGESTATE_FADEIN;
                return;
            }
            fadefx_out(image_rgb(0,0,0), 1.0);
            break;
        }

        /* fade-in effect (after playing a level) */
        case STAGESTATE_FADEIN: {
            fadefx_in(image_rgb(0,0,0), 1.0);
            state = STAGESTATE_NORMAL;
            break;
        }
    }
}



/*
 * stageselect_render()
 * Renders the scene
 */
void stageselect_render()
{
    int i;
    v2d_t cam = v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2);

    background_render_bg(bgtheme, cam);
    background_render_fg(bgtheme, cam);

    font_render(title, cam);
    font_render(msg, cam);
    font_render(page, cam);

    for(i=0; i<stage_count; i++) {
        if(i/STAGE_MAXPERPAGE == option/STAGE_MAXPERPAGE) {
            font_set_text(stage_label[i], (option==i) ? "<color=ffff00>%s - %s %d</color>" : "%s - %s %d", stage_data[i]->name, lang_get("STAGESELECT_ACT"), stage_data[i]->act);
            font_render(stage_label[i], cam);
        }
    }

    actor_render(icon, cam);
}






/* private methods */

/* loads the stage list from the level/ folder */
void load_stage_list()
{
    int i, j, deny_flags = FA_DIREC | FA_LABEL, c = 0;
    int max_paths;
    char path[] = "levels/*.lev";
    char abs_path[2][1024];

    logfile_message("load_stage_list()");

    /* official and $HOME files */
    absolute_filepath(abs_path[0], path, sizeof(abs_path[0]));
    home_filepath(abs_path[1], path, sizeof(abs_path[1]));
    max_paths = (strcmp(abs_path[0], abs_path[1]) == 0) ? 1 : 2;

    /* loading data */
    stage_count = 0;
    for(j=0; j<max_paths; j++)
        for_each_file_ex(abs_path[j], 0, deny_flags, dircount, NULL);

    stage_data = mallocx(stage_count * sizeof(stagedata_t*));
    for(j=0; j<max_paths; j++)
        for_each_file_ex(abs_path[j], 0, deny_flags, dirfill, (void*)&c);
    qsort(stage_data, stage_count, sizeof(stagedata_t*), sort_cmp);

    /* fatal error */
    if(stage_count == 0)
        fatal_error("FATAL ERROR: no level files were found! Please reinstall the game.");
    else
        logfile_message("%d levels found.", stage_count);

    /* other stuff */
    stage_label = mallocx(stage_count * sizeof(font_t**));
    for(i=0; i<stage_count; i++) {
        stage_label[i] = font_create(8);
        stage_label[i]->position = v2d_new(25, 60 + 20 * (i % STAGE_MAXPERPAGE));
    }
}



/* unloads the stage list */
void unload_stage_list()
{
    int i;

    logfile_message("unload_stage_list()");

    for(i=0; i<stage_count; i++) {
        font_destroy(stage_label[i]);
        stagedata_unload(stage_data[i]);
    }

    free(stage_label);
    free(stage_data);
    stage_count = 0;
}


/* callback that fills stage_data[] */
int dirfill(const char *filename, int attrib, void *param)
{
    int *c = (int*)param;
    int ver, subver, wipver;
    stagedata_t *s;

    s = stagedata_load(filename);
    if(s != NULL) {
        ver = s->requires[0];
        subver = s->requires[1];
        wipver = s->requires[2];

        if(game_version_compare(ver, subver, wipver) >= 0) {
            stage_data[ (*c)++ ] = s;
        }
        else {
            logfile_message("Warning: level file \"%s\" (requires: %d.%d.%d) isn't compatible with this version of the game (%d.%d.%d)", filename, ver, subver, wipver, GAME_VERSION, GAME_SUB_VERSION, GAME_WIP_VERSION);
            stagedata_unload(s);
        }
    }

    return 0;
}

/* callback that counts how many levels are installed */
int dircount(const char *filename, int attrib, void *param)
{
    int ver, subver, wipver;
    stagedata_t *s;

    s = stagedata_load(filename);
    if(s != NULL) {
        ver = s->requires[0];
        subver = s->requires[1];
        wipver = s->requires[2];

        if(game_version_compare(ver, subver, wipver) >= 0)
            stage_count++;

        stagedata_unload(s);
    }

    return 0;
}


/* comparator */
int sort_cmp(const void *a, const void *b)
{
    stagedata_t *s[2] = { *((stagedata_t**)a), *((stagedata_t**)b) };
    int r = str_icmp(s[0]->name, s[1]->name);
    return (r == 0) ? (s[0]->act - s[1]->act) : r;
}


/* stagedata_t constructor. Returns NULL if filename is not a level. */
stagedata_t* stagedata_load(const char *filename)
{
    parsetree_program_t *prog;
    stagedata_t* s = mallocx(sizeof *s);

    resource_filepath(s->filepath, (char*)filename, sizeof(s->filepath), RESFP_READ);
    strcpy(s->name, "Untitled");
    strcpy(s->bgtheme, "");
    s->act = 1;
    s->requires[0] = 0;
    s->requires[1] = 0;
    s->requires[2] = 0;

    prog = nanoparser_construct_tree(s->filepath);
    nanoparser_traverse_program_ex(prog, (void*)s, traverse);
    prog = nanoparser_deconstruct_tree(prog);

    /* invalid level! */
    if(s->requires[0] <= 0 && s->requires[1] <= 0 && s->requires[2] <= 0) {
        logfile_message("Warning: load_stage_data(\"%s\") - invalid level. filepath: \"%s\"", filename, s->filepath);
        free(s);
        s = NULL;
    }

    return s;
}

/* stagedata_t destructor */
void stagedata_unload(stagedata_t *s)
{
    free(s);
}

/* traverses a line of the level */
int traverse(const parsetree_statement_t *stmt, void *stagedata)
{
    stagedata_t *s = (stagedata_t*)stagedata;
    const char *id = nanoparser_get_identifier(stmt);
    const parsetree_parameter_t *param_list = nanoparser_get_parameter_list(stmt);
    const char *val = nanoparser_get_string(nanoparser_get_nth_parameter(param_list, 1));

    if(str_icmp(id, "name") == 0)
        str_cpy(s->name, val, sizeof(s->name));
    else if(str_icmp(id, "act") == 0)
        s->act = clip(atoi(val), 1, 3);
    else if(str_icmp(id, "requires") == 0)
        sscanf(val, "%d.%d.%d", &(s->requires[0]), &(s->requires[1]), &(s->requires[2]));
    else if(str_icmp(id, "bgtheme") == 0)
        str_cpy(s->bgtheme, val, sizeof(s->bgtheme));
    else if(str_icmp(id, "brick") == 0 || str_icmp(id, "item") == 0 || str_icmp(id, "enemy") == 0 || str_icmp(id, "object") == 0) /* optimization */
        return 1; /* stop the enumeration */

    return 0;
}
