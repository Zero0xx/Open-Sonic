/*
 * commandline.c - command line parser
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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "commandline.h"
#include "global.h"
#include "logfile.h"
#include "stringutil.h"
#include "util.h"
#include "osspec.h"
#include "video.h"
#include "lang.h"
#include "preferences.h"


/* private stuff ;) */
static void display_message(char *fmt, ...);



/*
 * commandline_parse()
 * Parses the command line arguments
 */
commandline_t commandline_parse(int argc, char **argv)
{
    int i;
    commandline_t cmd;

    /* default values */
    cmd.video_resolution = preferences_get_videoresolution();
    cmd.smooth_graphics = preferences_get_smooth();
    cmd.fullscreen = preferences_get_fullscreen();
    cmd.show_fps = preferences_get_showfps();
    str_cpy(cmd.language_filepath, preferences_get_languagepath(), sizeof(cmd.language_filepath));
    cmd.color_depth = max(16, desktop_color_depth());
    cmd.custom_level = FALSE;
    cmd.custom_quest = FALSE;

    /* logfile */
    logfile_message("game arguments:");
    for(i=0; i<argc; i++)
        logfile_message("argv[%d]: '%s'", i, argv[i]);

    /* reading data... */
    for(i=1; i<argc; i++) {

        if(str_icmp(argv[i], "--help") == 0) {
            display_message(
                "%s usage:\n"
                "    %s [options ...]\n"
                "\n"
                "where options include:\n"
                "    --help                    displays this message\n"
                "    --version                 shows the version of this program\n"
                "    --fullscreen              fullscreen mode\n"
                "    --resolution X            sets the window size, where X = TINY (%dx%d), NORMAL (%dx%d) or MAX (adapts to your desktop)\n"
                "    --smooth                  improves the graphic quality (*)\n"
                "    --tiny                    small game window (improves the speed **). This is the same as --resolution TINY\n"
                "    --show-fps                 shows the FPS (frames per second) counter\n"
                "    --level \"FILEPATH\"        runs the level located at FILEPATH\n"
                "    --quest \"FILEPATH\"        runs the quest located at FILEPATH\n"
                "    --color-depth X           sets the color depth to X bits/pixel, where X = 8, 16, 24 or 32\n"
                "    --language \"FILEPATH\"     sets the language file to FILEPATH (for example, %s)\n"
                "\n"
                "(*) This option may be used to improve the graphic quality using a special algorithm.\n"
                "    You should NOT use this option on slow computers, since it may imply a severe performance hit.\n"
                "\n"
                "(**) This option should be used on slow computers.\n"
                "\n"
                "Please read the user manual for more information.\n",
            GAME_TITLE, GAME_UNIXNAME,
            VIDEO_SCREEN_W, VIDEO_SCREEN_H, VIDEO_SCREEN_W*2, VIDEO_SCREEN_H*2,
            DEFAULT_LANGUAGE_FILEPATH);
            exit(0);
        }

        else if(str_icmp(argv[i], "--version") == 0) {
            display_message("%s %d.%d.%d\n", GAME_TITLE, GAME_VERSION, GAME_SUB_VERSION, GAME_WIP_VERSION);
            exit(0);
        }

        else if(str_icmp(argv[i], "--resolution") == 0) {
            if(++i < argc) {
                if(str_icmp(argv[i], "TINY") == 0)
                    cmd.video_resolution = VIDEORESOLUTION_1X;
                else if(str_icmp(argv[i], "NORMAL") == 0)
                    cmd.video_resolution = VIDEORESOLUTION_2X;
                else if(str_icmp(argv[i], "MAX") == 0)
                    cmd.video_resolution = VIDEORESOLUTION_MAX;
                else
                    display_message("WARNING: invalid resolution (%s).", argv[i]);
            }
        }

        else if(str_icmp(argv[i], "--smooth") == 0) {
            cmd.smooth_graphics = TRUE;
            if(cmd.video_resolution == VIDEORESOLUTION_1X)
                cmd.video_resolution = VIDEORESOLUTION_2X;
        }

        else if(str_icmp(argv[i], "--tiny") == 0)
            cmd.video_resolution = VIDEORESOLUTION_1X;

        else if(str_icmp(argv[i], "--fullscreen") == 0)
            cmd.fullscreen = TRUE;

        else if(str_icmp(argv[i], "--color-depth") == 0) {
            if(++i < argc) {
                cmd.color_depth = atoi(argv[i]);
                if(cmd.color_depth != 8 && cmd.color_depth != 16 && cmd.color_depth != 24 && cmd.color_depth != 32) {
                    display_message("WARNING: invalid color depth (%d). Changing to %d...\n", cmd.color_depth, 16);
                    cmd.color_depth = 16;
                }
            }
        }

        else if(str_icmp(argv[i], "--show-fps") == 0)
            cmd.show_fps = TRUE;

        else if(str_icmp(argv[i], "--level") == 0) {
            if(++i < argc) {
                cmd.custom_level = TRUE;
                resource_filepath(cmd.custom_level_path, argv[i], sizeof(cmd.custom_level_path), RESFP_READ);
                if(!filepath_exists(cmd.custom_level_path))
                    fatal_error("FATAL ERROR: file '%s' does not exist!\n", cmd.custom_level_path);
            }
        }

        else if(str_icmp(argv[i], "--quest") == 0) {
            if(++i < argc) {
                cmd.custom_quest = TRUE;
                resource_filepath(cmd.custom_quest_path, argv[i], sizeof(cmd.custom_quest_path), RESFP_READ);
                if(!filepath_exists(cmd.custom_quest_path))
                    fatal_error("FATAL ERROR: file '%s' does not exist!\n", cmd.custom_quest_path);
            }
        }

        else if(str_icmp(argv[i], "--language") == 0) {
            if(++i < argc) {
                resource_filepath(cmd.language_filepath, argv[i], sizeof(cmd.language_filepath), RESFP_READ);
                if(!filepath_exists(cmd.language_filepath))
                    fatal_error("FATAL ERROR: file '%s' does not exist!\n", cmd.language_filepath);
            }
        }

        else { /* unknown option */
            display_message("%s: bad command line option \"%s\".\nRun %s --help to get more information.\n", GAME_UNIXNAME, argv[i], GAME_UNIXNAME);
            exit(0);
        }

    }

    /* done! */
    return cmd;
}



/* private functions */


/*
 * display_message()
 * Displays a message (printf format)
 */
void display_message(char *fmt, ...)
{
    char buf[5120];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

#ifdef __WIN32__
    allegro_message(buf);
#else
    puts(buf);
#endif
}


