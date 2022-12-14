#
# CMakeLists.txt - tells CMake how to build the game
# Copyright (C) 2008-2010  Alexandre Martins <alemartf(at)gmail(dot)com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

# Initializing...
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(opensonic C)
SET(GAME_UNIXNAME opensonic)
SET(GAME_NAME "Open Sonic")
SET(GAME_VERSION "0.1.4")
SET(RTFM "Please read the user manual (readme.html) to get help.")
SET(ALLEGRO_RECOMMENDED_VERSION "4.4.1")
SET(CMAKE_BUILD_TYPE Release)
MESSAGE("${GAME_NAME} version ${GAME_VERSION}")

# configuring...
SET(DEFS "")
SET(CFLAGS_EXTRA "-g")
SET(CFLAGS "${CFLAGS} ${CMAKE_C_FLAGS}")
MESSAGE("Using CFLAGS='${CFLAGS}'")

IF(UNIX)
  SET(ALLEGRO_UNIX_LIBS "$ENV{_ALLEGRO_LIBS}")
  IF(NOT ALLEGRO_UNIX_LIBS)
    MESSAGE(FATAL_ERROR "Please do not invoke CMake directly. Run ./configure instead.")
  ENDIF(NOT ALLEGRO_UNIX_LIBS)
ENDIF(UNIX)





# Finding the required libraries:

# Allegro: liballeg
FIND_LIBRARY(LALLEG NAMES alleg PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LALLEG)
  MESSAGE(FATAL_ERROR "Fatal error: couldn't find the Allegro game programming library (liballeg)! ${RTFM}")
ELSE(NOT LALLEG)
  IF(UNIX)
    SET(ALLEGRO_UNIX_VERSION "$ENV{_ALLEGRO_VERSION}")
    IF(NOT ALLEGRO_UNIX_VERSION STREQUAL ALLEGRO_RECOMMENDED_VERSION)
      MESSAGE("\nWARNING: your Allegro version is ${ALLEGRO_UNIX_VERSION}. The recommended version is ${ALLEGRO_RECOMMENDED_VERSION}.\n         ${GAME_NAME} may not compile properly!.\n")
    ENDIF(NOT ALLEGRO_UNIX_VERSION STREQUAL ALLEGRO_RECOMMENDED_VERSION)
  ENDIF(UNIX)
ENDIF(NOT LALLEG)

# loadpng: libloadpng, libpng and zlib
FIND_LIBRARY(LLOADPNG NAMES loadpng PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LLOADPNG)
  MESSAGE(FATAL_ERROR "Fatal error: libloadpng not found! ${RTFM}")
ENDIF(NOT LLOADPNG)

FIND_LIBRARY(LPNG NAMES png PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LPNG)
  MESSAGE(FATAL_ERROR "Fatal error: libpng not found! ${RTFM}")
ENDIF(NOT LPNG)

FIND_LIBRARY(LZ NAMES z PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LZ)
  MESSAGE(FATAL_ERROR "Fatal error: zlib not found! ${RTFM}")
ENDIF(NOT LZ)

# JPGalleg: libjpgalleg
FIND_LIBRARY(LJPGALLEG NAMES jpgalleg PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LJPGALLEG)
  MESSAGE(FATAL_ERROR "Fatal error: libjpgalleg not found! ${RTFM}")
ENDIF(NOT LJPGALLEG)

# LOGG: liblogg, libogg, libvorbis, libvorbisfile
FIND_LIBRARY(LLOGG NAMES logg PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LLOGG)
  MESSAGE(FATAL_ERROR "Fatal error: liblogg not found! ${RTFM}")
ENDIF(NOT LLOGG)

FIND_LIBRARY(LOGG NAMES ogg PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LOGG)
  MESSAGE(FATAL_ERROR "Fatal error: libogg not found! ${RTFM}")
ENDIF(NOT LOGG)

FIND_LIBRARY(LVORBIS NAMES vorbis PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LVORBIS)
  MESSAGE(FATAL_ERROR "Fatal error: libvorbis not found! ${RTFM}")
ENDIF(NOT LVORBIS)

FIND_LIBRARY(LVORBISFILE NAMES vorbisfile PATH "${CMAKE_LIBRARY_PATH}")
IF(NOT LVORBISFILE)
  MESSAGE(FATAL_ERROR "Fatal error: libvorbisfile not found! ${RTFM}")
ENDIF(NOT LVORBISFILE)


# RC compiler
IF(NOT CMAKE_RC_COMPILER)
  SET(CMAKE_RC_COMPILER windres)
ENDIF(NOT CMAKE_RC_COMPILER)


# pre-processor: #define's
IF(MSVC)
  FOREACH(d ${DEFS})
    SET(CFLAGS_EXTRA "${CFLAGS_EXTRA} /D${d}")
  ENDFOREACH(d)
ELSE(MSVC)
  FOREACH(d ${DEFS})
    SET(CFLAGS_EXTRA "${CFLAGS_EXTRA} -D${d}")
  ENDFOREACH(d)
ENDIF(MSVC)


# source files
SET(
  GAME_SRCS

  src/core/2xsai/2xsai.c
  src/core/nanoparser/nanoparser.c
  src/core/audio.c
  src/core/commandline.c
  src/core/engine.c
  src/core/image.c
  src/core/input.c
  src/core/lang.c
  src/core/logfile.c
  src/core/osspec.c
  src/core/preferences.c
  src/core/quest.c
  src/core/resourcemanager.c
  src/core/scene.c
  src/core/screenshot.c
  src/core/soundfactory.c
  src/core/sprite.c
  src/scenes/stageselect.c
  src/core/storyboard.c
  src/core/stringutil.c
  src/core/timer.c
  src/core/util.c
  src/core/v2d.c
  src/core/video.c

  src/scenes/util/editorgrp.c
  src/scenes/util/grouptree.c
  src/scenes/confirmbox.c
  src/scenes/credits.c
  src/scenes/endofdemo.c
  src/scenes/gameover.c
  src/scenes/intro.c
  src/scenes/langselect.c
  src/scenes/level.c
  src/scenes/menu.c
  src/scenes/options.c
  src/scenes/pause.c
  src/scenes/quest.c
  src/scenes/questover.c

  src/entities/object_decorators/base/objectbasicmachine.c
  src/entities/object_decorators/base/objectdecorator.c
  src/entities/object_decorators/add_rings.c
  src/entities/object_decorators/add_to_score.c
  src/entities/object_decorators/attach_to_player.c
  src/entities/object_decorators/audio.c
  src/entities/object_decorators/bounce_player.c
  src/entities/object_decorators/bullet_trajectory.c
  src/entities/object_decorators/enemy.c
  src/entities/object_decorators/change_closest_object_state.c
  src/entities/object_decorators/children.c
  src/entities/object_decorators/clear_level.c
  src/entities/object_decorators/create_item.c
  src/entities/object_decorators/destroy.c
  src/entities/object_decorators/elliptical_trajectory.c
  src/entities/object_decorators/gravity.c
  src/entities/object_decorators/jump.c
  src/entities/object_decorators/lock_camera.c
  src/entities/object_decorators/look.c
  src/entities/object_decorators/mosquito_movement.c
  src/entities/object_decorators/move_player.c
  src/entities/object_decorators/on_event.c
  src/entities/object_decorators/hit_player.c
  src/entities/object_decorators/player_movement.c
  src/entities/object_decorators/player_action.c
  src/entities/object_decorators/set_alpha.c
  src/entities/object_decorators/set_animation.c
  src/entities/object_decorators/set_obstacle.c
  src/entities/object_decorators/set_player_speed.c
  src/entities/object_decorators/set_player_animation.c
  src/entities/object_decorators/set_player_position.c
  src/entities/object_decorators/dialog_box.c
  src/entities/object_decorators/walk.c
  src/entities/object_decorators/observe_player.c

  src/entities/items/util/itemutil.c
  src/entities/items/animal.c
  src/entities/items/animalprison.c
  src/entities/items/bigring.c
  src/entities/items/bluering.c
  src/entities/items/bumper.c
  src/entities/items/checkpointorb.c
  src/entities/items/crushedbox.c
  src/entities/items/danger.c
  src/entities/items/dangpower.c
  src/entities/items/dnadoor.c
  src/entities/items/door.c
  src/entities/items/endsign.c
  src/entities/items/explosion.c
  src/entities/items/falglasses.c
  src/entities/items/fireball.c
  src/entities/items/flyingtext.c
  src/entities/items/goalsign.c
  src/entities/items/icon.c
  src/entities/items/itembox.c
  src/entities/items/loop.c
  src/entities/items/ring.c
  src/entities/items/spikes.c
  src/entities/items/spring.c
  src/entities/items/switch.c
  src/entities/items/teleporter.c

  src/entities/actor.c
  src/entities/background.c
  src/entities/boss.c
  src/entities/brick.c
  src/entities/camera.c
  src/entities/enemy.c
  src/entities/object_compiler.c
  src/entities/object_vm.c
  src/entities/font.c
  src/entities/item.c
  src/entities/player.c

  src/main.c
)



# *nix executable
IF(UNIX)
  SET(GAME_SRCS ${GAME_SRCS} src/misc/iconlin.c)
  ADD_EXECUTABLE(${GAME_UNIXNAME} ${GAME_SRCS})
  SET_TARGET_PROPERTIES(${GAME_UNIXNAME} PROPERTIES LINK_FLAGS ${ALLEGRO_UNIX_LIBS})
  TARGET_LINK_LIBRARIES(${GAME_UNIXNAME} m logg vorbisfile vorbis ogg jpgalleg z png loadpng)
  SET_TARGET_PROPERTIES(${GAME_UNIXNAME} PROPERTIES COMPILE_FLAGS "-Wall -O2 ${CFLAGS} ${CFLAGS_EXTRA}")
ENDIF(UNIX)



# Windows executable
IF(WIN32)

  # MSVC fix
  IF(MSVC)
    SET(
      GAME_SRCS
      ${GAME_SRCS}

      src/core/2xsai/2xsai.h
      src/core/nanoparser/nanoparser.h
      src/core/audio.h
      src/core/commandline.h
      src/core/engine.h
      src/core/global.h
      src/core/hashtable.h
      src/core/image.h
      src/core/input.h
      src/core/lang.h
      src/core/logfile.h
      src/core/osspec.h
      src/core/preferences.h
      src/core/quest.h
      src/core/resourcemanager.h
      src/core/scene.h
      src/core/screenshot.h
      src/core/soundfactory.h
      src/core/sprite.h
      src/core/storyboard.h
      src/core/stringutil.h
      src/core/timer.h
      src/core/util.h
      src/core/video.h
      src/core/v2d.h

      src/scenes/util/editorgrp.h
      src/scenes/util/grouptree.h
      src/scenes/confirmbox.h
      src/scenes/credits.h
      src/scenes/endofdemo.h
      src/scenes/gameover.h
      src/scenes/intro.h
      src/scenes/langselect.h
      src/scenes/level.h
      src/scenes/menu.h
      src/scenes/options.h
      src/scenes/pause.h
      src/scenes/quest.h
      src/scenes/questover.h
      src/scenes/stageselect.h

      src/entities/object_decorators/base/objectbasicmachine.h
      src/entities/object_decorators/base/objectdecorator.h
      src/entities/object_decorators/base/objectmachine.h
      src/entities/object_decorators/add_rings.h
      src/entities/object_decorators/add_to_score.h
      src/entities/object_decorators/attach_to_player.h
      src/entities/object_decorators/audio.h
      src/entities/object_decorators/bounce_player.h
      src/entities/object_decorators/bullet_trajectory.h
      src/entities/object_decorators/enemy.h
      src/entities/object_decorators/change_closest_object_state.h
      src/entities/object_decorators/children.h
      src/entities/object_decorators/clear_level.h
      src/entities/object_decorators/create_item.h
      src/entities/object_decorators/destroy.h
      src/entities/object_decorators/elliptical_trajectory.h
      src/entities/object_decorators/gravity.h
      src/entities/object_decorators/jump.h
      src/entities/object_decorators/lock_camera.h
      src/entities/object_decorators/look.h
      src/entities/object_decorators/mosquito_movement.h
      src/entities/object_decorators/move_player.h
      src/entities/object_decorators/on_event.h
      src/entities/object_decorators/hit_player.h
      src/entities/object_decorators/player_movement.h
      src/entities/object_decorators/player_action.h
      src/entities/object_decorators/set_alpha.h
      src/entities/object_decorators/set_animation.h
      src/entities/object_decorators/set_obstacle.h
      src/entities/object_decorators/set_player_speed.h
      src/entities/object_decorators/set_player_animation.h
      src/entities/object_decorators/set_player_position.h
      src/entities/object_decorators/dialog_box.h
      src/entities/object_decorators/walk.h
      src/entities/object_decorators/observe_player.h

      src/entities/items/util/itemutil.h
      src/entities/items/animal.h
      src/entities/items/animalprison.h
      src/entities/items/bigring.h
      src/entities/items/bluering.h
      src/entities/items/bumper.h
      src/entities/items/checkpointorb.h
      src/entities/items/crushedbox.h
      src/entities/items/danger.h
      src/entities/items/dangpower.h
      src/entities/items/dnadoor.h
      src/entities/items/door.h
      src/entities/items/endsign.h
      src/entities/items/explosion.h
      src/entities/items/falglasses.h
      src/entities/items/fireball.h
      src/entities/items/flyingtext.h
      src/entities/items/goalsign.h
      src/entities/items/icon.h
      src/entities/items/itembox.h
      src/entities/items/loop.h
      src/entities/items/ring.h
      src/entities/items/spikes.h
      src/entities/items/spring.h
      src/entities/items/switch.h
      src/entities/items/teleporter.h

      src/entities/actor.h
      src/entities/background.h
      src/entities/boss.h
      src/entities/brick.h
      src/entities/camera.h
      src/entities/enemy.h
      src/entities/object_compiler.h
      src/entities/object_vm.h
      src/entities/font.h
      src/entities/item.h
      src/entities/player.h

      src/misc/iconwin.rc
    )
  ENDIF(MSVC)

  # Executables
  ADD_EXECUTABLE(${GAME_UNIXNAME} WIN32 ${GAME_SRCS})

  # Other properties
  IF(MSVC)
    SET_TARGET_PROPERTIES(${GAME_UNIXNAME} PROPERTIES COMPILE_FLAGS "/D_CRT_SECURE_NO_DEPRECATE /D__WIN32__ /D__MSVC__ ${CFLAGS} ${CFLAGS_EXTRA}")
    TARGET_LINK_LIBRARIES(${GAME_UNIXNAME} logg vorbisfile vorbis ogg jpgalleg loadpng alleg png z)
  ELSE(MSVC)
    SET_TARGET_PROPERTIES(${GAME_UNIXNAME} PROPERTIES COMPILE_FLAGS "-Wall -O2 -ansi ${CFLAGS} ${CFLAGS_EXTRA}")
    TARGET_LINK_LIBRARIES(${GAME_UNIXNAME} m logg vorbisfile vorbis ogg jpgalleg loadpng alleg png z)
    EXECUTE_PROCESS(COMMAND ${CMAKE_RC_COMPILER} -O coff -o src/misc/iconwin.res -i src/misc/iconwin.rc)
    SET_TARGET_PROPERTIES(${GAME_UNIXNAME} PROPERTIES LINK_FLAGS "src/misc/iconwin.res")
  ENDIF(MSVC)
ENDIF(WIN32)



# Misc
SET_TARGET_PROPERTIES(${GAME_UNIXNAME} PROPERTIES PROJECT_NAME "${GAME_NAME}")



# Installing on *nix
IF(UNIX)
  SET(GAME_INSTALL_DIR "/usr/share/${GAME_UNIXNAME}")
  SET(GAME_FINAL_DIR "/usr/bin")

  INSTALL(CODE "MESSAGE(\"Installing ${GAME_NAME} ${GAME_VERSION}... Make sure you have root privileges.\")")
  INSTALL(TARGETS ${GAME_UNIXNAME} RUNTIME DESTINATION ${GAME_INSTALL_DIR})
  INSTALL(FILES license.txt readme.html DESTINATION ${GAME_INSTALL_DIR})
  INSTALL(DIRECTORY objects sprites config images levels licenses musics quests samples screenshots themes languages DESTINATION ${GAME_INSTALL_DIR} PATTERN ".svn" EXCLUDE)

  INSTALL(CODE "MESSAGE(\"Creating files at ${GAME_FINAL_DIR}...\")")
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND \"cmake\" \"-E\" \"copy\" \"${GAME_INSTALL_DIR}/${GAME_UNIXNAME}\" \"${GAME_FINAL_DIR}/${GAME_UNIXNAME}\")")

  INSTALL(CODE "MESSAGE(\"Done! Please run ${GAME_UNIXNAME} to start ${GAME_NAME}.\")")
ENDIF(UNIX)
