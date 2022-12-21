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
#include <dirent.h>
#include <time.h>
#include <pigpio.h>
#include <pigpiod_if2.h>		// stupid pigpio cannot be run without beig root...
#ifndef WIN32
#include <unistd.h>
#endif
#include <jack/jack.h>
#include <jack/midiport.h>
#include <libconfig.h>
#include <fluidsynth.h>

/* default GPIO pins */
#define LED_GPIO	19
#define SWITCH_GPIO	26
#define ANTIBOUNCE_US   250000      // 0.25 sec = 250000 usec : used for switch anti-bouncing check : allows 240BPM max
#define TIMEON_US       200000      // 0.20 sec : used as on/off time for leds 

/* default soundfont file */
#define DEFAULT_SF2 "./soundfonts/00_FluidR3_GM.sf2"

/* constants */
#define MIDI_CLOCK 0xF8
#define MIDI_RESERVED 0xF9
#define MIDI_PLAY 0xFA
#define MIDI_STOP 0xFC
#define MIDI_CLOCK_RATE 96 // 24*4 ticks for full note, 24 ticks per quarter note

#define NB_NAMES 2		// 2 file names: 1 midi file name, 1 SF2 file name
#define FIRST_ELT 0		// used for declarations and loops for filename struct
#define	B0	0
#define	B1	1
#define	B2	2
#define	B3	3
#define	B4	4
#define	B5	5
#define	B6	6
#define	B7	7
#define PLAY	8		// play file
#define LOAD	9		// load files (midi and SF2) to player
#define LAST_ELT 10		// used for declarations and loops

#define NB_FCT 1		// 1 line of functions for midi file: vol -/+, BPM -/+
#define FIRST_ELT_FCT 0		// used for declarations and loops for function struct
#define	VOLDOWN	0
#define	VOLUP	1
#define	BPMDOWN	2
#define	BPMUP	3
#define BEAT	4
#define LAST_ELT_FCT 5		// used for declarations and loops

/* define status, etc */
#define TRUE 1
#define FALSE 0

#define NAMES 0
#define FCT 1

#define CLOCK_PLAY_READY 3
#define	CLOCK_PLAY 2
#define CLOCK 1
#define NO_CLOCK 0

#define FIRST_STATE 0		// used for declarations and loops
#define OFF 0
#define ON 1
#define PENDING	2
#define LAST_STATE 3		// used for declarations and loops

/* list management (used for led mgmt) */
#define LIST_ELT 100


/* types */
typedef struct {						// structure for each of the 2 names
	unsigned char ctrl [LAST_ELT] [2];	//controls on the midi control surface
	unsigned char led [LAST_ELT] [LAST_STATE] [3];		// led lightings on the midi control surface (off, pending, on...)
	unsigned char status [LAST_ELT];	// Status byte for each function
} filename_t;

typedef struct {						// structure for each of the 2 names
	unsigned char ctrl [LAST_ELT_FCT] [2];	//controls on the midi control surface
	unsigned char led [LAST_ELT_FCT] [LAST_STATE] [3];		// led lightings on the midi control surface (off, pending, on...)
	unsigned char status [LAST_ELT_FCT];	// Status byte for each function
} filefunct_t;

