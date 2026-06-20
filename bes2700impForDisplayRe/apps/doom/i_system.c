/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2006 by Colin Phipps, Florian Schulze
 *
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Misc system stuff needed by Doom, implemented for POSIX systems.
 *  Timers and signals.
 *
 *-----------------------------------------------------------------------------
 */

#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>

#include "doomtype.h"
#include "m_fixed.h"
#include "i_system.h"
#include "doomdef.h"
#include "lprintf.h"
#include "d_main.h"
#include "d_event.h"
#include "global_data.h"
#include "tables.h"
#include "input.h"
#include "bes_compat.h"

#include "i_system.h"

#include "global_data.h"
#include "hid_ev.h"

void I_Quit()
{
	while (1)
		;
}
/* cphipps - I_GetVersionString
 * Returns a version string in the given buffer
 */
const char *I_GetVersionString(char *buf, size_t sz)
{
	sprintf(buf, "PocketDoom v%s", VERSION);
	return buf;
}

#define MAX_MESSAGE_SIZE 1024

void I_Error(const char *error, ...)
{
	char msg[MAX_MESSAGE_SIZE];

	va_list v;
	va_start(v, error);

	vsprintf(msg, error, v);

	va_end(v);

	printf("%s", msg);

	fflush(stderr);
	fflush(stdout);

	// fgets(msg, sizeof(msg), stdin);

	I_Quit();
}

void initTheCustomIO()
{
}

// Originale, physikalische Bildschirmauflösung
#define SCREEN_WIDTH 192
#define SCREEN_HEIGHT 490

// Tastenbelegung für Aktionen (g_game.c: key_fire=KEYD_B, key_use/menu_enter=KEYD_A,
// key_escape/menu_escape=KEYD_START, menu nav = arrows).
#define KEY_UP KEYD_UP
#define KEY_DOWN KEYD_DOWN
#define KEY_LEFT KEYD_LEFT
#define KEY_RIGHT KEYD_RIGHT
#define KEY_FIRE KEYD_B      // shoot
#define KEY_USE  KEYD_A      // open doors / switches + menu confirm (enter)
#define KEY_MENU KEYD_START  // escape: open/close menu (and menu "back")

// Tap-Zonen-Grenzen (Panel 212x520, Doom-Bild 1:1 zentriert -> x46-165, y180-339):
//   y<ZONE_TOP_Y      -> START/Menü (ganz oben)
//   above/below image -> vorwärts / rückwärts
//   left/right of img -> links / rechts drehen
//   on image (centre) -> Feuer (+Use)
#define IMG_X0      46
#define IMG_X1      165
#define IMG_Y0      180
#define IMG_Y1      339
#define ZONE_TOP_Y  60
#define PANEL_MID   106   // splits the area below the image into BACK (left) / USE (right)

// Empfindlichkeit
#define SWIPE_DEADZONE 40 // Pixel-Distanz, die eine Berührung zurücklegen muss, um als "Swipe" zu gelten

// --- GLOBALE VARIABLEN FÜR DEN TOUCH-ZUSTAND ---
static uint16_t touch_x = 0;
static uint16_t touch_y = 0;
static boolean is_currently_touched = false; // Der wahre Zustand, der über Zyklen hinweg erhalten bleibt

// Speichert den Zustand der Aktionen zwischen den Funktionsaufrufen
static int action_up_active = 0;
static int action_down_active = 0;
static int action_left_active = 0;
static int action_right_active = 0;
static int action_fire_active = 0;
static int action_use_active = 0;
static int action_menu_active = 0;

// ZUSTANDSVERWALTUNG FÜR GESTEN
typedef enum {
    GESTURE_NONE,       // Keine Berührung aktiv
    GESTURE_PENDING,    // Berührung gestartet, warte auf Entscheidung (Tap oder Swipe)
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT
} GestureState;

static GestureState current_gesture = GESTURE_NONE;
static int touch_start_x = 0;
static int touch_start_y = 0;

// Countdown-Zähler für Aktionen (a tap holds the key for a few frames)
static int fire_hold_counter = 0;
static int use_hold_counter = 0;
static int menu_hold_counter = 0;

/**
 * @brief Liest neue Touch-Daten, wenn verfügbar, und aktualisiert den globalen Gesten-Zustand.
 */
/* BES2700 port: read the Hynitron CST92xx via the firmware's touch driver
 * (platform/main/hw_selftest.c). Panel coords: X<212, Y<520. ev: 0x06 down,
 * 0x07 move, 0x00 up. Feeds the gesture state machine in I_ProcessKeyEvents. */
extern int doom_touch_poll(int *x, int *y, int *ev);

void read_touchscreen(void)
{
    int x = 0, y = 0, ev = 0;
    int down = doom_touch_poll(&x, &y, &ev);

    if (down && ev != 0x00)          /* 0x06 down / 0x07 move */
    {
        touch_x = (uint16_t)x;
        touch_y = (uint16_t)y;
        if (!is_currently_touched)   /* first contact -> start a gesture */
        {
            is_currently_touched = true;
            current_gesture = GESTURE_PENDING;
            touch_start_x = touch_x;
            touch_start_y = touch_y;
        }
    }
    else                             /* lift / no finger */
    {
        is_currently_touched = false;
    }
}

/**
 * @brief Sendet ein Tastenereignis an die DOOM-Engine und loggt die Aktion.
 */
void post_key_event(int key, int is_down, int *current_state_ptr, const char* key_name)
{
    if (is_down && !(*current_state_ptr))
    {
        event_t ev = {.type = ev_keydown, .data1 = key};
        D_PostEvent(&ev);
        *current_state_ptr = 1;
        am_util_stdio_printf("EVENT: Sent KEYDOWN for %s\r\n", key_name);
    }
    else if (!is_down && *current_state_ptr)
    {
        event_t ev = {.type = ev_keyup, .data1 = key};
        D_PostEvent(&ev);
        *current_state_ptr = 0;
        am_util_stdio_printf("EVENT: Sent KEYUP for %s\r\n", key_name);
    }
}


// --- HAUPTFUNKTION ZUR EVENT-VERARBEITUNG ---

void I_ProcessKeyEvents(void)
{
	// 1. Prüfen, ob neue Touch-Daten da sind und den globalen Zustand aktualisieren
	read_touchscreen();

	// 2. Flags für Aktionen in diesem Frame initialisieren
	int wants_up = 0;
	int wants_down = 0;
	int wants_left = 0;
	int wants_right = 0;
	int wants_fire = 0;
	int wants_use = 0;
	int wants_menu = 0;

	// 3. Direkte Tap-Zonen: die Taste ist aktiv, SOLANGE der Finger in der Zone
	//    liegt -> Halten = Dauerbewegung / Dauerfeuer (kein Swipe mehr).
	if (is_currently_touched)
	{
		int x = touch_x, y = touch_y;
		if (y < ZONE_TOP_Y)                             // ganz oben: zwei Tasten nebeneinander
		{
			if (x < PANEL_MID)       wants_menu = 1;    //   links  = START/Menü (Escape)
			else                     wants_use  = 1;    //   rechts = USE (Menü-Enter, Spiel starten, Türen)
		}
		else if (y < IMG_Y0)         wants_up    = 1;   // über dem Bild  = vorwärts / Menü hoch
		else if (y > IMG_Y1)         wants_down  = 1;   // unter dem Bild = rückwärts / Menü runter
		else if (x < IMG_X0)         wants_left  = 1;   // links vom Bild = links drehen
		else if (x > IMG_X1)         wants_right = 1;   // rechts vom Bild = rechts drehen
		else                         wants_fire  = 1;   // Bildmitte = Feuer
	}

	// 5. Events für Bewegung und Aktionen an DOOM senden.
	post_key_event(KEY_UP,    wants_up,    &action_up_active,    "UP");
	post_key_event(KEY_DOWN,  wants_down,  &action_down_active,  "DOWN");
	post_key_event(KEY_LEFT,  wants_left,  &action_left_active,  "LEFT");
	post_key_event(KEY_RIGHT, wants_right, &action_right_active, "RIGHT");
	post_key_event(KEY_FIRE,  wants_fire,  &action_fire_active,  "FIRE");
	post_key_event(KEY_USE,   wants_use,   &action_use_active,   "USE");
	post_key_event(KEY_MENU,  wants_menu,  &action_menu_active,  "MENU");
}