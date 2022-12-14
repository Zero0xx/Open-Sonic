/*
 * video.c - video manager
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

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <png.h>
#include <allegro.h>
#include <loadpng.h>
#include <jpgalleg.h>
#include "2xsai/2xsai.h"
#include "video.h"
#include "timer.h"
#include "logfile.h"
#include "util.h"




/* private data */


/* video manager */
#define FILTER_2XSAI            0
#define FILTER_SUPEREAGLE       1
static image_t *video_buffer;
static image_t *window_surface, *window_surface_half;
static int video_smooth;
static int video_resolution;
static int video_fullscreen;
static int video_showfps;
static void fast2x_blit(image_t *src, image_t *dest);
static void filter_blit(image_t *src, image_t *dest, int filter);
static void window_switch_in();
static void window_switch_out();
static int window_active = TRUE;
static void losangle(image_t *img, int x, int y, int r, int col);
static void draw_to_screen(image_t *img);
static void setup_color_depth(int bpp);

/* Fade-in & fade-out */
#define FADEFX_NONE            0
#define FADEFX_IN              1
#define FADEFX_OUT             2
static int fadefx_type;
static int fadefx_end;
static uint32 fadefx_color;
static float fadefx_elapsed_time;
static float fadefx_total_time;

/* Video Message */
#define VIDEOMSG_TIMEOUT    5000
static uint32 videomsg_endtime;
static char videomsg_data[512];



/* video manager */

/*
 * video_init()
 * Initializes the video manager
 */
void video_init(const char *window_title, int resolution, int smooth, int fullscreen, int bpp)
{
    logfile_message("video_init()");
    setup_color_depth(bpp);

    /* initializing addons */
    logfile_message("Initializing JPGalleg...");
    jpgalleg_init();
    logfile_message("Initializing loadpng...");
    loadpng_init();

    /* video init */
    video_buffer = NULL;
    window_surface = window_surface_half = NULL;
    video_changemode(resolution, smooth, fullscreen);

    /* window properties */
    LOCK_FUNCTION(game_quit);
    set_close_button_callback(game_quit);
    set_window_title(window_title);

    /* window callbacks */
    window_active = TRUE;
    if(set_display_switch_mode(SWITCH_BACKGROUND) == 0) {
        if(set_display_switch_callback(SWITCH_IN, window_switch_in) != 0)
            logfile_message("can't set_display_switch_callback(SWTICH_IN, window_switch_in)");

        if(set_display_switch_callback(SWITCH_OUT, window_switch_out) != 0)
            logfile_message("can't set_display_switch_callback(SWTICH_OUT, window_switch_out)");
    }
    else
        logfile_message("can't set_display_switch_mode(SWITCH_BACKGROUND)");

    /* video message */
    videomsg_endtime = 0;
}

/*
 * video_changemode()
 * Sets up the game window
 */
void video_changemode(int resolution, int smooth, int fullscreen)
{
    int width, height;
    int mode;

    logfile_message("video_changemode(%d,%d,%d)", resolution, smooth, fullscreen);

    /* resolution */
    video_resolution = resolution;

    /* fullscreen */
    video_fullscreen = fullscreen;

    /* smooth graphics? */
    video_smooth = smooth;
    if(video_smooth) {
        if(video_get_color_depth() == 8) {
            logfile_message("can't use smooth graphics in the 256 color mode (8 bpp)");
            video_smooth = FALSE;
        }
        else if(video_resolution == VIDEORESOLUTION_1X || video_resolution == VIDEORESOLUTION_EDT) {
            logfile_message("can't use smooth graphics using resolution %d", video_resolution);
            video_smooth = FALSE;
        }
        else {
            logfile_message("initializing 2xSaI...");
            Init_2xSaI(video_get_color_depth());
        }
    }

    /* creating the backbuffer... */
    logfile_message("creating the backbuffer...");
    if(video_buffer != NULL)
        image_destroy(video_buffer);
    video_buffer = image_create(VIDEO_SCREEN_W, VIDEO_SCREEN_H);
    image_clear(video_buffer, image_rgb(0,0,0));

    /* creating the window surface... */
    logfile_message("creating the window surface...");
    if(window_surface != NULL)
        image_destroy(window_surface);
    window_surface = image_create((int)(video_get_window_size().x), (int)(video_get_window_size().y));
    image_clear(window_surface, image_rgb(0,0,0));

    logfile_message("creating the auxiliary window surface...");
    if(window_surface_half != NULL)
        image_destroy(window_surface_half);
    window_surface_half = image_create(window_surface->w/2, window_surface->h/2);
    image_clear(window_surface_half, image_rgb(0,0,0));

    /* setting up the window... */
    logfile_message("setting up the window...");
    mode = video_fullscreen ? GFX_AUTODETECT : GFX_AUTODETECT_WINDOWED;
    width = (int)(video_get_window_size().x);
    height = (int)(video_get_window_size().y);
    if(set_gfx_mode(mode, width, height, 0, 0) < 0)
        fatal_error("video_changemode(): couldn't set the graphic mode!\n%s", allegro_error);

    /* done! */
    logfile_message("video_changemode() ok");
}


/*
 * video_get_resolution()
 * Returns the current resolution value,
 * i.e., VIDEORESOLUTION_*
 */
int video_get_resolution()
{
    return video_resolution;
}


/*
 * video_is_smooth()
 * Smooth graphics?
 */
int video_is_smooth()
{
    return video_smooth;
}


/*
 * video_is_fullscreen()
 * Fullscreen mode?
 */
int video_is_fullscreen()
{
    return video_fullscreen;
}


/*
 * video_get_window_size()
 * Returns the window size, based on
 * the current resolution
 */
v2d_t video_get_window_size()
{
    int width=VIDEO_SCREEN_W, height=VIDEO_SCREEN_H;
    int dw, dh; /* desktop resolution */

    switch(video_resolution) {
        case VIDEORESOLUTION_1X:
            width = VIDEO_SCREEN_W;
            height = VIDEO_SCREEN_H;
            break;

        case VIDEORESOLUTION_2X:
            width = 2*VIDEO_SCREEN_W;
            height = 2*VIDEO_SCREEN_H;
            break;

        case VIDEORESOLUTION_MAX:
            if(get_desktop_resolution(&dw, &dh) == 0) {
                int scale = min((int)(dw/VIDEO_SCREEN_W), (int)(dh/VIDEO_SCREEN_H));
                width = scale*VIDEO_SCREEN_W;
                height = scale*VIDEO_SCREEN_H;
            }
            else {
                width = VIDEO_SCREEN_W;
                height = VIDEO_SCREEN_H;
            }
            break;

        case VIDEORESOLUTION_EDT:
            width = VIDEO_SCREEN_W;
            height = VIDEO_SCREEN_H;
            break;

        default:
            fatal_error("video_get_window_size(): unknown resolution!");
            break;
    }

    return v2d_new(width, height);
}


/*
 * video_get_backbuffer()
 * Returns a pointer to the backbuffer
 */
image_t* video_get_backbuffer()
{
    if(video_buffer == NULL)
        fatal_error("FATAL ERROR: video_get_backbuffer() returned NULL!");

    return video_buffer;
}

/*
 * video_render()
 * Updates the video manager and the screen
 */
void video_render()
{
    /* fade effect */
    fadefx_end = FALSE;
    if(fadefx_type != FADEFX_NONE) {
        fadefx_elapsed_time += timer_get_delta();
        if(fadefx_elapsed_time < fadefx_total_time) {
            if(video_get_color_depth() > 8) {
                /* true-color fade effect */
                int n;

                n = (int)( (float)255 * (fadefx_elapsed_time*1.25 / fadefx_total_time) );
                n = clip(n, 0, 255);
                n = (fadefx_type == FADEFX_IN) ? 255-n : n;

                drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
                set_trans_blender(0, 0, 0, n);
                rectfill(video_get_backbuffer()->data, 0, 0, VIDEO_SCREEN_W, VIDEO_SCREEN_H, fadefx_color);
                solid_mode();
            }
            else {
                /* 256-color fade effect */
                int i, j, x, y, r;
                float prob;

                prob = fadefx_elapsed_time / fadefx_total_time;
                prob = (fadefx_type == FADEFX_IN) ? 1.0 - prob : prob;

                for(i=0; i<=20; i++) {
                    for(j=0; j<=10; j++) {
                        r = (int)( ((1.0-(float)i/20)/8 + 7*prob/8)*50.0 );
                        x = (int)((float)i/20 * video_get_backbuffer()->w);
                        y = (int)((float)j/10 * video_get_backbuffer()->h);
                        losangle(video_get_backbuffer(), x, y, r, fadefx_color);
                    }
                }
            }
        }
        else {
            if(fadefx_type == FADEFX_OUT)
                rectfill(video_get_backbuffer()->data, 0, 0, VIDEO_SCREEN_W, VIDEO_SCREEN_H, fadefx_color);
            fadefx_type = FADEFX_NONE;
            fadefx_total_time = fadefx_elapsed_time = 0;
            fadefx_color = 0;
            fadefx_end = TRUE;
        }
    }



    /* video message */
    if(timer_get_ticks() < videomsg_endtime)
        textout_ex(video_get_backbuffer()->data, font, videomsg_data, 0, VIDEO_SCREEN_H-text_height(font), makecol(255,255,255), makecol(0,0,0));


    /* fps counter */
    if(video_is_fps_visible())
        textprintf_right_ex(video_get_backbuffer()->data, font, VIDEO_SCREEN_W, 0, makecol(255,255,255), makecol(0,0,0),"FPS: %d", timer_get_fps());


    /* render */
    switch(video_get_resolution()) {
        /* tiny window */
        case VIDEORESOLUTION_1X:
        {
            draw_to_screen(video_get_backbuffer());
            break;
        }

        /* double size */
        case VIDEORESOLUTION_2X:
        {
            image_t *tmp = window_surface;

            if(video_is_smooth())
                filter_blit(video_get_backbuffer(), tmp, FILTER_2XSAI);
            else
                fast2x_blit(video_get_backbuffer(), tmp);

            draw_to_screen(tmp);
            break;
        }

        /* maximum size */
        case VIDEORESOLUTION_MAX:
        {
            image_t *tmp = window_surface;

            if(video_is_smooth() && tmp->w >= 2*VIDEO_SCREEN_W && tmp->h >= 2*VIDEO_SCREEN_H) {
                image_t *half = window_surface_half;
                v2d_t scale = v2d_new((float)half->w / (float)video_get_backbuffer()->w, (float)half->h / (float)video_get_backbuffer()->h);
                image_draw_scaled(video_get_backbuffer(), half, 0, 0, scale, IF_NONE);
                filter_blit(half, tmp, FILTER_2XSAI);
            }
            else {
                v2d_t scale = v2d_new((float)tmp->w / (float)video_get_backbuffer()->w, (float)tmp->h / (float)video_get_backbuffer()->h);
                image_draw_scaled(video_get_backbuffer(), tmp, 0, 0, scale, IF_NONE);
            }

            draw_to_screen(tmp);
            break;
        }

        /* editor */
        case VIDEORESOLUTION_EDT:
        {
            draw_to_screen(video_get_backbuffer());
            break;
        }
    }
}


/*
 * video_release()
 * Releases the video manager
 */
void video_release()
{
    logfile_message("video_release()");

    if(video_buffer != NULL)
        image_destroy(video_buffer);

    if(window_surface != NULL)
        image_destroy(window_surface);

    if(window_surface_half != NULL)
        image_destroy(window_surface_half);

    logfile_message("video_release() ok");
}


/*
 * video_showmessage()
 * Shows a text message to the user
 */
void video_showmessage(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsprintf(videomsg_data, fmt, args);
    va_end(args);

    videomsg_endtime = timer_get_ticks() + VIDEOMSG_TIMEOUT;
}


/*
 * video_get_color_depth()
 * Returns the current color depth
 */
int video_get_color_depth()
{
    return get_color_depth();
}


/*
 * video_is_window_active()
 * Returns TRUE if the game window is active,
 * or FALSE otherwise
 */
int video_is_window_active()
{
    return window_active;
}



/*
 * video_get_maskcolor()
 * Returns the mask color
 */
uint32 video_get_maskcolor()
{
    switch(video_get_color_depth()) {
        case 8:  return MASK_COLOR_8;
        case 16: return MASK_COLOR_16;
        case 24: return MASK_COLOR_24;
        case 32: return MASK_COLOR_32;
    }

    return MASK_COLOR_16;
}


/*
 * video_show_fps()
 * Shows/hides the FPS counter
 */
void video_show_fps(int show)
{
    video_showfps = show;
}


/*
 * video_is_fps_visible()
 * Is the FPS counter visible?
 */
int video_is_fps_visible()
{
    return video_showfps;
}





/* private stuff */

/* filter_blit() applies the graphic filter
 * filter, fixing the possible defects of the
 * resulting image.
 *
 * if (filter == 2xsai) or (filter == superagle):
 * -- we assume that:
 * ---- dest->w = 2 * src->w
 * ---- dest->h = 2 * src->h
 */
void filter_blit(image_t *src, image_t *dest, int filter)
{
    int i, j, k=2;

    if(src->data == NULL || dest->data == NULL)
        return;

    switch(filter) {
        case FILTER_2XSAI:
            Super2xSaI(src->data, dest->data, 0, 0, 0, 0, src->w, src->h);
            for(i=0; i<dest->h; i++) { /* image fix */
                for(j=0; j<k; j++)
                    putpixel(dest->data, j, i, getpixel(dest->data, k, i));
            }
            break;

        case FILTER_SUPEREAGLE:
            SuperEagle(src->data, dest->data, 0, 0, 0, 0, src->w, src->h);
            for(i=0; i<dest->h; i++) { /* image fix */
                for(j=0; j<k; j++)
                    putpixel(dest->data, dest->w-1-j, i, getpixel(dest->data, dest->w-1-k, i));
            }
            break;
    }
}

/* fast2x_blit resizes the src image by a
 * factor of 2. It assumes that:
 *
 * src is a memory bitmap
 * dest is a previously created memory bitmap
 * dest->w == 2 * src->w
 * dest->h == 2 * src->h */
void fast2x_blit(image_t *src, image_t *dest)
{
    int i, j;

    if(src->data == NULL || dest->data == NULL)
        return;

    switch(video_get_color_depth())
    {
        case 8:
            for(j=0; j<dest->h; j++) {
                for(i=0; i<dest->w; i++)
                    ((uint8*)dest->data->line[j])[i] = ((uint8*)src->data->line[j/2])[i/2];
            }
            break;

        case 16:
            for(j=0; j<dest->h; j++) {
                for(i=0; i<dest->w; i++)
                    ((uint16*)dest->data->line[j])[i] = ((uint16*)src->data->line[j/2])[i/2];
            }
            break;

        case 24:
            /* TODO */
            stretch_blit(src->data, dest->data, 0, 0, src->w, src->h, 0, 0, dest->w, dest->h);
            break;

        case 32:
            for(j=0; j<dest->h; j++) {
                for(i=0; i<dest->w; i++)
                    ((uint32*)dest->data->line[j])[i] = ((uint32*)src->data->line[j/2])[i/2];
            }
            break;

        default:
            break;
    }
}


/* draws img to the screen */
void draw_to_screen(image_t *img)
{
    if(img->data == NULL) {
        logfile_message("Can't use video resolution %d", video_get_resolution());
        video_showmessage("Can't use video resolution %d", video_get_resolution());
        video_changemode(VIDEORESOLUTION_1X, video_is_smooth(), video_is_fullscreen());
    }
    else
        blit(img->data, screen, 0, 0, 0, 0, img->w, img->h);
}

/* this window is active */
void window_switch_in()
{
    window_active = TRUE;
}


/* this window is not active */
void window_switch_out()
{
    window_active = FALSE;
}

/* losangle */
void losangle(image_t *img, int x, int y, int r, int col)
{
    int points[] = {
        x,   y-r,
        x-r, y,
        x,   y+r,
        x+r, y
    };

    polygon(img->data, 4, points, col);
}

/* setups the color depth */
void setup_color_depth(int bpp)
{
    set_color_depth(bpp);

    if(bpp == 8)
        set_color_conversion(COLORCONV_REDUCE_TO_256 | COLORCONV_DITHER_PAL);
    else
        set_color_conversion(COLORCONV_TOTAL);
}



/* fade effects */

/*
 * fadefx_in()
 * Fade-in effect
 */
void fadefx_in(uint32 color, float seconds)
{
    if(fadefx_type == FADEFX_NONE) {
        fadefx_type = FADEFX_IN;
        fadefx_end = FALSE;
        fadefx_color = color;
        fadefx_elapsed_time = 0;
        fadefx_total_time = seconds;
    }
}


/*
 * fadefx_out()
 * Fade-out effect
 */
void fadefx_out(uint32 color, float seconds)
{
    if(fadefx_type == FADEFX_NONE) {
        fadefx_type = FADEFX_OUT;
        fadefx_end = FALSE;
        fadefx_color = color;
        fadefx_elapsed_time = 0;
        fadefx_total_time = seconds;
    }
}



/*
 * fadefx_over()
 * Asks if the fade effect has ended
 * (only one action when this event loops)
 */
int fadefx_over()
{
    return fadefx_end;
}


/*
 * fadefx_is_fading()
 * Is the fade effect ocurring?
 */
int fadefx_is_fading()
{
    return (fadefx_type != FADEFX_NONE);
}

