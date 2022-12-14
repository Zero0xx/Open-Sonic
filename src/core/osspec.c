/*
 * osspec.c - OS Specific Routines
 * Copyright (C) 2009-2010  Alexandre Martins <alemartf(at)gmail(dot)com>
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

#include <stdio.h>
#include <allegro.h>
#include "global.h"
#include "osspec.h"
#include "util.h"
#include "stringutil.h"



/* uncomment to disable case insensitive filename support
 * on platforms that do not support it (like *nix)
 * (see fix_case_path() for more information).
 *
 * Disabling this feature can speed up a little bit
 * the file searching routines on platforms like *nix
 * (it has no effect under Windows, though).
 *
 * Keeping this enabled provides case insensitive filename
 * support on platforms like *nix. */
/*#define DISABLE_FIX_CASE_PATH*/



/* uncomment to disable filepath optimizations - useful
 * on distributed file systems (example: nfs).
 *
 * Disabling this feature improves memory usage just
 * a little bit. You probably want to keep this
 * option enabled.
 *
 * Keeping this enabled improves drastically the
 * speed of the game when the files are stored
 * in a distributed file system like NFS. */
/*#define DISABLE_FILEPATH_OPTIMIZATIONS*/




#ifndef __WIN32__

#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#else

#include <winalleg.h>

#endif




/* private stuff */
#ifndef __WIN32__
static struct passwd *userinfo;
#endif
static char* fix_case_path(char *filepath);
static int fix_case_path_backtrack(const char *pwd, const char *remaining_path, const char *delim, char *dest);
static char executable_name[1024];
static void search_the_file(char *dest, const char *relativefp, size_t dest_size);

#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
/* cache stuff (also private): it's a basic dictionary */
static void cache_init();
static void cache_release();
static char *cache_search(const char *key);
static void cache_insert(const char *key, char *value);

/* the cache is implemented as a simple binary tree */
typedef struct cache_t {
    char *key, *value;
    struct cache_t *left, *right;
} cache_t;
static cache_t *cache_root;
static cache_t *cachetree_release(cache_t *node);
static cache_t *cachetree_search(cache_t *node, const char *key);
static cache_t *cachetree_insert(cache_t *node, const char *key, const char *value);
#endif


/* public functions */

/* 
 * osspec_init()
 * Operating System Specifics - initialization
 */
void osspec_init()
{
#ifndef __WIN32__
    int i;
    char tmp[1024];
    char subdirs[][32] = {
        { "" },              /* $HOME/.$GAME_UNIXNAME/ */
        { "levels" },        /* $HOME/.$GAME_UNIXNAME/levels */
        { "screenshots" },   /* etc. */
        { "mods" },
        { "themes" },
        { "quests"}
    }; /* TODO: quest '.sav' directory? */



    /* retrieving user data */
    if(NULL == (userinfo = getpwuid(getuid())))
        fprintf(stderr, "WARNING: couldn't obtain information about your user. User-specific data may not work.\n");



    /* creating sub-directories */
    for(i=0; i<sizeof(subdirs)/32; i++) {
        home_filepath(tmp, subdirs[i], sizeof(tmp));
        mkdir(tmp, 0755);
    }
#endif

    /* executable name */
    get_executable_name(executable_name, sizeof(executable_name));

#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
    /* initializing the cache */
    cache_init();
#endif
}


/* 
 * osspec_release()
 * Operating System Specifics - release
 */
void osspec_release()
{
#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
    /* releasing the cache */
    cache_release();
#endif
}


/*
 * filepath_exists()
 * Returns TRUE if the given file exists
 * or FALSE otherwise
 */
int filepath_exists(const char *filepath)
{
    return exists(filepath);
}



/*
 * directory_exists()
 * Returns TRUE if the given directory exists
 * or FALSE otherwise
 */
int directory_exists(const char *dirpath)
{
    return file_exists(dirpath, FA_DIREC | FA_HIDDEN | FA_RDONLY, NULL);
}




/*
 * absolute_filepath()
 * Converts a relative filepath into an
 * absolute filepath.
 */
void absolute_filepath(char *dest, const char *relativefp, size_t dest_size)
{
    if(is_relative_filename(relativefp)) {
#ifndef __WIN32__
        char tmp[1024];
        str_cpy(tmp, executable_name, sizeof(tmp));
        tmp[ strlen(GAME_UNIX_COPYDIR) ] = '\0';
        if(strcmp(tmp, GAME_UNIX_COPYDIR) == 0)
            sprintf(dest, "%s/%s", GAME_UNIX_INSTALLDIR, relativefp);
        else {
            str_cpy(dest, executable_name, dest_size);
            replace_filename(dest, dest, relativefp, dest_size);
        }
#else
        str_cpy(dest, executable_name, dest_size);
        replace_filename(dest, dest, relativefp, dest_size);
#endif
    }
    else
        str_cpy(dest, relativefp, dest_size); /* relativefp is already an absolute filepath */

    fix_filename_slashes(dest);
    canonicalize_filename(dest, dest, dest_size);
    fix_case_path(dest);
}



/*
 * home_filepath()
 * Similar to absolute_filepath(), but this routine considers
 * the $HOME/.$GAME_UNIXNAME/ directory instead
 */
void home_filepath(char *dest, const char *relativefp, size_t dest_size)
{
#ifndef __WIN32__

    if(userinfo) {
        sprintf(dest, "%s/.%s/%s", userinfo->pw_dir, GAME_UNIXNAME, relativefp);
        fix_filename_slashes(dest);
        canonicalize_filename(dest, dest, dest_size);
        fix_case_path(dest);
    }
    else
        absolute_filepath(dest, relativefp, dest_size);

#else

    absolute_filepath(dest, relativefp, dest_size);

#endif
}




/*
 * resource_filepath()
 * Similar to absolute_filepath() and home_filepath(), but this routine
 * searches the specified file both in the home directory and in the
 * game directory
 */
void resource_filepath(char *dest, const char *relativefp, size_t dest_size, int resfp_mode)
{
    switch(resfp_mode) {
        /* I'll read the file */
        case RESFP_READ:
        {

#ifndef DISABLE_FILEPATH_OPTIMIZATIONS

            /* optimizations: without this, the game could become terribly slow
             * when the files are distributed over a network (example: nfs) */
            char *path;
            if(is_relative_filename(relativefp)) {
                if(NULL == (path=cache_search(relativefp))) {
                    /* I'll have to search the file... */
                    search_the_file(dest, relativefp, dest_size);

                    /* store the resulting filepath in the memory */
                    path = mallocx( (strlen(dest)+1) * sizeof *path );
                    strcpy(path, dest);
                    cache_insert(relativefp, path);
                }
                else
                    str_cpy(dest, path, dest_size);
            }
            else
                search_the_file(dest, relativefp, dest_size);

#else

            search_the_file(dest, relativefp, dest_size);

#endif
            break;

        }

        /* I'll write to the file */
        case RESFP_WRITE:
        {
            FILE *fp;
            struct al_ffblk info;
            absolute_filepath(dest, relativefp, dest_size);

            if(al_findfirst(dest, &info, FA_HIDDEN) == 0) {
                /* the file exists AND it's NOT read-only */
                al_findclose(&info);
            }
            else {

                /* the file does not exist OR it is read-only */
                if(!filepath_exists(dest)) {

                    /* it doesn't exist */
                    if(NULL != (fp = fopen(dest, "w"))) {
                        /* is it writable? */
                        fclose(fp);
                        delete_file(dest);
                    }
                    else {
                        /* it's not writable */
                        home_filepath(dest, relativefp, dest_size);
                    }

                }
                else {
                    /* the file exists, but it's read-only */
                    home_filepath(dest, relativefp, dest_size);
                }

            }

            break;
        }

        /* Unknown mode */
        default:
        {
            fprintf(stderr, "resource_filepath(): invalid resfp_mode (%d)", resfp_mode);
            break;
        }
    }
}



/*
 * create_process()
 * Creates a new process;
 *     path is the absolute path to the executable file
 *     argc is argument count
 *     argv[] contains the command line arguments
 *
 * NOTE: argv[0] also contains the absolute path of the executable
 */
void create_process(const char *path, int argc, char *argv[])
{
#ifndef __WIN32__
    pid_t pid;

    argv[argc] = NULL;
    if(0 == (pid=fork())) /* if(child process) */
        execv(path, argv);
#else
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char cmd[10240]="";
    int i, is_file;

    for(i=0; i<argc; i++) {
        is_file = filepath_exists(argv[i]); /* TODO: test folders with spaces (must test program.exe AND level.lev) */
        if(is_file) strcat(cmd, "\"");
        strcat(cmd, argv[i]);
        strcat(cmd, is_file ? "\" " : " ");
    }
    cmd[strlen(cmd)-1] = '\0'; /* erase the last blank space */

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if(!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        MessageBox(NULL, "Couldn't CreateProcess()", "Error", MB_OK);
        MessageBox(NULL, cmd, "Command Line", MB_OK);
        exit(1);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif
}


/*
 * basename()
 * Finds out the filename portion of a completely specified file path
 */
char *basename(const char *path)
{
    return get_filename(path);
}









/* private methods */

/* backtracking routine used in fix_case_path()
 * returns TRUE iff a solution is found */
int fix_case_path_backtrack(const char *pwd, const char *remaining_path, const char *delim, char *dest)
{
    char *query, *pos, *p, *tmp, *new_pwd;
    const char *q;
    struct al_ffblk info;
    int ret = FALSE;

    if(NULL != (pos = strchr(remaining_path, *delim))) {
        /* if remaning_path is "my/example/query", then
           query is equal to "my" */
        query = mallocx(sizeof(char) * (pos-remaining_path+1));
        for(p=query,q=remaining_path; q!=pos;)
            *p++ = *q++;
        *p = 0;

        /* next folder... */
        tmp = mallocx(sizeof(char) * (strlen(pwd)+2));
        sprintf(tmp, "%s*", pwd);
        if(al_findfirst(tmp, &info, FA_ALL) == 0) {
            do {
                if(str_icmp(query, info.name) == 0) {
                    new_pwd = mallocx(sizeof(char) * (strlen(pwd)+strlen(query)+strlen(delim)+1));
                    sprintf(new_pwd, "%s%s%s", pwd, info.name, delim);
                    ret = fix_case_path_backtrack(new_pwd, pos+1, delim, dest);
                    free(new_pwd);
                    if(ret) break;
                }
            }
            while(al_findnext(&info) == 0);
            al_findclose(&info);
        }
        free(tmp);

        /* releasing resources */
        free(query);
    }
    else {
        /* no more subdirectories */
        tmp = mallocx(sizeof(char) * (strlen(pwd)+2));
        sprintf(tmp, "%s*", pwd);
        if(al_findfirst(tmp, &info, FA_ALL) == 0) {
            do {
                if(str_icmp(remaining_path, info.name) == 0) {
                    ret = TRUE;
                    sprintf(dest, "%s%s", pwd, info.name);
                    break;
                }
            }
            while(al_findnext(&info) == 0);
            al_findclose(&info);
        }
        free(tmp);
    }

    /* done */
    return ret;
}


/* Case-insensitive filename support for all platforms.
 *
 * If the user requests for the file "LEVELS/MyLevel.lev", but
 * only "levels/mylevel.lev" exists, the valid filepath will
 * be used. This routine does nothing on Windows.
 *
 * filepath may be modified during the process. A copy of it
 * is returned. */
char* fix_case_path(char *filepath)
{
#if !defined(DISABLE_FIX_CASE_PATH) && !defined(__WIN32__)
    char *tmp;
    const char delimiter[] = "/";
    int solved = FALSE;

    if(!filepath_exists(filepath)) {
        fix_filename_slashes(filepath);
        tmp = mallocx(sizeof(char) * (strlen(filepath)+1));

        if(*filepath == *delimiter)
            solved = fix_case_path_backtrack(delimiter, filepath+1, delimiter, tmp);
        else
            solved = fix_case_path_backtrack("", filepath, delimiter, tmp);

        if(solved)
            strcpy(filepath, tmp);

        free(tmp);
    }
#endif

    return filepath;
}


/* auxiliary routine for resource_filepath(): given any filepath (relative or absolute),
 * finds the absolute path (either in the home directory or in the game directory) */
void search_the_file(char *dest, const char *relativefp, size_t dest_size)
{
    home_filepath(dest, relativefp, dest_size);
    if(!filepath_exists(dest) && !directory_exists(dest))
        absolute_filepath(dest, relativefp, dest_size);
}





#ifndef DISABLE_FILEPATH_OPTIMIZATIONS
/* ------- cache interface -------- */

/* initializes the cache */
void cache_init()
{
    cache_root = NULL;
}

/* releases the cache */
void cache_release()
{
    cache_root = cachetree_release(cache_root);
}

/* finds a string in the dictionary */
char *cache_search(const char *key)
{
    cache_t *node = cachetree_search(cache_root, key);
    return node ? node->value : NULL;
}

/* inserts a string into the dictionary */
void cache_insert(const char *key, char *value)
{
    cache_root = cachetree_insert(cache_root, key, value);
}


/* ------ cache implementation --------- */
cache_t *cachetree_release(cache_t *node)
{
    if(node) {
        node->left = cachetree_release(node->left);
        node->right = cachetree_release(node->right);
        free(node->key);
        free(node->value);
        free(node);
    }

    return NULL;
}

cache_t *cachetree_search(cache_t *node, const char *key)
{
    int cmp;

    if(node) {
        cmp = strcmp(key, node->key);

        if(cmp < 0)
            return cachetree_search(node->left, key);
        else if(cmp > 0)
            return cachetree_search(node->right, key);
        else
            return node;
    }
    else
        return NULL;
}

cache_t *cachetree_insert(cache_t *node, const char *key, const char *value)
{
    int cmp;
    cache_t *t;

    if(node) {
        cmp = strcmp(key, node->key);

        if(cmp < 0)
            return (node->left = cachetree_insert(node->left, key, value));
        else if(cmp > 0)
            return (node->right = cachetree_insert(node->right, key, value));
        else
            return node;
    }
    else {
        t = mallocx(sizeof *t);
        t->key = mallocx(sizeof *(t->key) * (strlen(key)+1));
        t->value = mallocx(sizeof *(t->value) * (strlen(value)+1));
        t->left = t->right = NULL;
        strcpy(t->key, key);
        strcpy(t->value, value);
        return t;
    }
}
#endif

