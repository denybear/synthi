/** @file types.h
 *
 * @brief This file defines constants and main global types
 *
 */

/* includes */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <jack/jack.h>
#include <jack/midiport.h>
#include <libconfig.h>
#include <fluidsynth.h>



/* constants */
#define NB_TRACKS	6	// number of tracks for the looper
#define NB_BAR_ROWS 2	// number of bar rows to select tehe number of bars to record
#define MIDI_CLOCK 0xF8
#define MIDI_RESERVED 0xF9
#define MIDI_PLAY 0xFA
#define MIDI_STOP 0xFC
#define MIDI_CLOCK_RATE 96 // 24*4 ticks for full note, 24 ticks per quarter note

#define FIRST_ELT 0		// used for declarations and loops
#define TIMESIGN 0
#define	LOAD 1
#define	SAVE 2
#define PLAY 3
#define RECORD 4
#define MUTE 5
#define SOLO 6
#define VOLDOWN 7
#define VOLUP 8
#define MODE 9
#define DELETE 10
#define LAST_ELT 11		// used for declarations and loops

#define LAST_BAR_ELT 8		// used for declarations and loops


/* max number of samples of each track buffer (L,R) */
#define NB_SAMPLES	13230000	// 13230000 samples at 44100 Hz means 300 seconds of music, ie. 5 min loops

/* time signature values */
#define FIRST_TIMESIGN 0
#define _4_4 0
#define _2_2 1
#define	_2_4 2
#define	_3_4 3
#define	_6_8 4
#define	_9_8 5
#define	_12_8 6
#define	_5_4 7
#define LAST_TIMESIGN 7
// let's not support 3_8 nor 6_4 for now at it messes with our BBT calculations algorithm

/* define status, etc */
#define TRUE 1
#define FALSE 0

#define	CLOCK_PLAY 2
#define CLOCK 1
#define NO_CLOCK 0


#define TRACK 0
#define BAR 1

#define ON_BBT 2

#define FIRST_STATE 0		// used for declarations and loops
#define OFF 0
#define ON 1
#define PENDING_ON 2
#define PENDING_OFF 3
#define LAST_STATE 4		// used for declarations and loops

/* list management (used for led mgmt) */
#define LIST_ELT 100

