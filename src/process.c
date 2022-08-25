/** @file process.c
 *
 * @brief The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 */

#include "types.h"
#include "globals.h"
#include "config.h"
#include "process.h"
#include "utils.h"
#include "led.h"


// number of midi clock signal sent per quarter note; from 0 to 23
int midi_clock_num;
// previous index pulse: used to see whether we changed quarter note (beat)
int previous_index_pulse;


// main process callback called at capture of (nframes) frames/samples
int process ( jack_nframes_t nframes, void *arg )
{
	jack_nframes_t h;
	int i,j,k;
	int mute = OFF;
	void *midiin;
	void *midiout;
	void *clockout;
	jack_midi_event_t in_event;
	jack_midi_data_t buffer[5];				// midi out buffer for lighting the pad leds and for midi clock
	int dest, row, col, on_off;				// variables used to manage lighting of the pad leds



	/**************************************/
	/* First, process MIDI in (UI) events */
	/**************************************/

	// Get midi clock midi out and midi in buffers
	midiin = jack_port_get_buffer(midi_input_port, nframes);

	// process MIDI IN events
	for (i=0; i< jack_midi_get_event_count(midiin); i++) {
		if (jack_midi_event_get (&in_event, midiin, i) != 0) {
			fprintf ( stderr, "Missed in event\n" );
			continue;
		}
		// call processing function
		midi_in_process (&in_event,nframes);
	}


	/*****************************************/
	/* Second, process MIDI CLOCK out events */
	/*****************************************/

	// define midi out port to write to
	clockout = jack_port_get_buffer (clock_output_port, nframes);

	// clear midi write buffer
	jack_midi_clear_buffer (clockout);

	// start of the song
	// check if we should send midi PLAY (+ midi clock right next)
	if (send_clock == CLOCK_PLAY) {
		buffer [0] = MIDI_PLAY;
		jack_midi_event_write (clockout, 0, buffer, 1);
		// send MIDI_CLOCK right after MIDI_PLAY by setting send_clock
		send_clock = CLOCK;
	}
	
	// check if we should send midi clock
	if (send_clock == CLOCK) {
		buffer [0] = MIDI_CLOCK;
		jack_midi_event_write (clockout, 0, buffer, 1);
		// clear send_clock flag; it will be set back again by tick handling process
		send_clock = NO_CLOCK;
	}


	/***************************************/
	/* Third, process MIDI out (UI) events */
	/***************************************/

	// define midi out port to write to
	midiout = jack_port_get_buffer (midi_output_port, nframes);

	// clear midi write buffer
	jack_midi_clear_buffer (midiout);

	//go through the list of led requests
	while (pull_from_list(&dest, &row, &col, &on_off)) {

		// if this is a filename led
		// copy midi event required to light led into midi buffer
		if (dest == NAMES) memcpy (buffer, &filename[row].led [col][on_off][0], 3);
		// copy midi event required to light led into midi buffer
		if (dest == FCT) memcpy (buffer, &filefunct[row].led [col][on_off][0], 3);

		// if buffer is not empty, then send as midi out event
		// we take care of writing led events at different time marks to make sure all of these are taken into account
		if (buffer [0] | buffer [1] | buffer [2]) {
			jack_midi_event_write (midiout, 0, buffer, 3);
		}
	}


	return 0;
}


// process callback called to process midi_in events in realtime
int midi_in_process (jack_midi_event_t *event, jack_nframes_t nframes) {

	int i,j;
	int k,l;


	// check if play pad has been pressed
	if (same_event(event->buffer,filename[0].ctrl[PLAY])) {
		// toggle is_play value from ON to OFF (TRUE to FALSE)
		is_play = (is_play == TRUE) ? FALSE : TRUE;

		// test if playing shall be started or stopped
		if (is_play) {
			// init clock sending to indicate PLAY has been pressed
			send_clock = CLOCK_PLAY_READY;
			// rewind to the beggining of the file
			fluid_player_seek (player, 0);
			// play the midi files, if any
			if (fluid_player_play (player) == FLUID_FAILED) {
				// no file to play; force is_play to FALSE
				is_play = FALSE;
			}
		}
		else
		{
			// stop the midi files, if any
			fluid_player_stop (player);
		}

		// set play led according to play value
		led_filename (0, PLAY, is_play);
	}


	// check if "load files" pad has been pressed
	if (same_event(event->buffer,filename[0].ctrl[LOAD])) {
		// set LOAD value to TRUE; it will be set back to false in the main thread, when files are actually loaded 
		is_load=TRUE;
		// set load led according to load value
		led_filename (0, LOAD, is_load);
	}


	// check if midi filename pads have been pressed
	// check midi file name and SF2 file name
	for (i=0; i<NB_NAMES; i++) {
		// check all bits in name, and set status accordingly
		for (j=B0; j<B7+1; j++) {
			if (same_event(event->buffer,filename[i].ctrl[j])) {
				// if status == 1 (ON), then set to 0 (OFF); if == 0, set to 1
				filename[i].status[j] = (filename[i].status[j] == ON) ? OFF : ON;
				// light/unlight corresponding led
				led_filename (i, j, filename[i].status[j]);
			}
		}
	}
	
	// PROCESS FILE FUNCTIONS : VOL -/+, BPM -/+
	// check if volume down pad has been pressed
	if (same_event(event->buffer,filefunct[0].ctrl[VOLDOWN])) {
		// set VOLUME value to TRUE; it will be set back to false in the main thread, where volume is actually set
		is_volume=TRUE;
		// adjust volume: decrements until is reaches 0
		volume = (volume <= 0) ? 0 : (volume - 1);

		// if volume == 0, then light on volume down pad to indicate we have reached the lower limit
		if (volume == 0) {
				led_filefunct (0, VOLDOWN, ON);
				led_filefunct (0, VOLUP, OFF);
		}
		else {
			// if volume == 2 (default value), then light on both pads, in PENDING mode
			if (volume == 2) {
				led_filefunct (0, VOLDOWN, PENDING);
				led_filefunct (0, VOLUP, PENDING);
			}
			// in other cases, turn light of both pads (voldown and up)
			else {
				led_filefunct (0, VOLDOWN, OFF);
				led_filefunct (0, VOLUP, OFF);
			}
		}
	}

	// check if volume up pad has been pressed
	if (same_event(event->buffer,filefunct[0].ctrl[VOLUP])) {
		// set VOLUME value to TRUE; it will be set back to false in the main thread, where volume is actually set
		is_volume=TRUE;
		// adjust volume: increments until is reaches 1
		volume = (volume >= 10) ? 10 : (volume + 1);

		// if volume == 10, then light on volume up pad to indicate we have reached the higher limit
		if (volume == 10) {
				led_filefunct (0, VOLDOWN, OFF);
				led_filefunct (0, VOLUP, ON);
		}
		else {
			
			// if volume == 2 (default value), then light on both pads, in PENDING mode
			if (volume == 2) {
				led_filefunct (0, VOLDOWN, PENDING);
				led_filefunct (0, VOLUP, PENDING);
			}
			// in other cases, turn light of both pads (voldown and up)
			else {
				led_filefunct (0, VOLDOWN, OFF);
				led_filefunct (0, VOLUP, OFF);
			}
		}
	}

	// check if BPM down pad has been pressed
	if (same_event(event->buffer,filefunct[0].ctrl[BPMDOWN])) {

		// get initial BPM, in case we don't have it yet
		if (initial_bpm == -1) {
			initial_bpm = (fluid_player_get_bpm (player) == FLUID_FAILED) ? 0 : fluid_player_get_bpm (player);
		}

		// get bpm of the file
		bpm = (fluid_player_get_bpm (player) == FLUID_FAILED) ? 0 : fluid_player_get_bpm (player);

		// adjust tempo: decrements until is reaches 0
		bpm = (bpm <= 0) ? 0 : (bpm - 2);
		fluid_player_set_tempo (player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, bpm);

		// if bpm == 0, then light on bpm down pad to indicate we have reached the lower limit
		if (bpm == 0) {
				led_filefunct (0, BPMDOWN, ON);
				led_filefunct (0, BPMUP, OFF);
		}
		else {
			// if bpm == initial bpm of the file, then light on both pads, in PENDING mode
			if (bpm == initial_bpm) {
				led_filefunct (0, BPMDOWN, PENDING);
				led_filefunct (0, BPMUP, PENDING);
			}
			// in other cases, turn light off on both pads (bpmdown and up)
			else {
				led_filefunct (0, BPMDOWN, OFF);
				led_filefunct (0, BPMUP, OFF);
			}
		}
	}

	// check if BPM up pad has been pressed
	if (same_event(event->buffer,filefunct[0].ctrl[BPMUP])) {

		// get initial BPM, in case we don't have it yet
		if (initial_bpm == -1) {
			initial_bpm = (fluid_player_get_bpm (player) == FLUID_FAILED) ? 0 : fluid_player_get_bpm (player);
		}

		// get bpm of the file
		bpm = (fluid_player_get_bpm (player) == FLUID_FAILED) ? 0 : fluid_player_get_bpm (player);

		// adjust tempo: increments until it reaches 60000000
		bpm = (bpm >= 60000000) ? 60000000 : (bpm + 2);
		fluid_player_set_tempo (player, FLUID_PLAYER_TEMPO_EXTERNAL_BPM, bpm);

		// if bpm == 60000000, then light on bpm up pad to indicate we have reached the higher limit
		if (bpm == 60000000) {
				led_filefunct (0, BPMDOWN, OFF);
				led_filefunct (0, BPMUP, ON);
		}
		else {
			// if bpm == initial bpm of the file, then light on both pads, in PENDING mode
			if (bpm == initial_bpm) {
				led_filefunct (0, BPMDOWN, PENDING);
				led_filefunct (0, BPMUP, PENDING);
			}
			// in other cases, turn light off on both pads (bpmdown and up)
			else {
				led_filefunct (0, BPMDOWN, OFF);
				led_filefunct (0, BPMUP, OFF);
			}
		}
	}
	
}


// process callback called to process midi player ticks in realtime
// we use this function to convert midi file ticks (PPQ ticks per quarrter note)
// into MIDI_CLOCK ticks (24 ticks per quarter note) to drive external midi systems
int handle_tick(void *data, int tick) {

	fluid_player_t* player;
	int ppq;
	int index_pulse;
	int i;
	float ppq_per_midi_clock;
	int end_tick;


	// define data as being a pointer to player
	player = (fluid_player_t*) data;
	// pulse per quarter note
	ppq = fluid_player_get_division (player);
	// number of pulse per midi_clock event
	ppq_per_midi_clock = ppq / 24.0;
	// index of pulse within quarter note
	index_pulse = tick % ppq;

	// make sure the song ends on a exact beat... not in the middle of a beat
	// division shall be integer division so remaining is lost and we have an exact multiple of ppq 
	end_tick = (fluid_player_get_total_ticks (player) + 1) / ppq;
	end_tick *= ppq; 	// here, end_tick shall contain the tick value of real song end
	// we should not send clock signals if tick is greater or equal to end_tick... so we stop at end of last beat, and could loop properly
	if (tick >= end_tick) {
		// in any case, set previous pulse index to current
		previous_index_pulse = index_pulse;
		return FLUID_OK;
	}
	
/* for debug purpose only
char s[80];
char s2 [50];
sprintf (s, "tick:%d - ppq:%d - index_pulse:%d", tick, ppq, index_pulse);
*/

	// Check if PLAY has just been pressed
	if (send_clock == CLOCK_PLAY_READY) {
		send_clock = CLOCK_PLAY;
		// let's send first MIDI_CLOCK signal of the beat
		midi_clock_num = 1;
		// set previous_index_pulse to 0, as this is the start of the song
		// at start of song, we send MIDI_PLAY then MIDI_CLOCK
		// hence by setting to 0, we indicate we have sent MIDI_CLOCK already
		previous_index_pulse = 0;

/* for debug purpose only
trace_beat = 1;
sprintf (s2," - CLOCK: %02d - START", trace_beat);
strcat (s, s2);
*/

	}
	else {
		// check if we changed beat; if yes, send CLOCK signal
		// for this, we check whether previous pulse index is greater than current
		// if greater, it means we changed beat
		if (previous_index_pulse > index_pulse) {
			// we are at the start of a new beat
			// let's send first MIDI_CLOCK signal of the beat
			midi_clock_num = 1;
			send_clock = CLOCK;

/* for debug purpose only
trace_beat = 1;
sprintf (s2," - CLOCK: %02d - BEAT", trace_beat);
strcat (s, s2);
*/

		}
		
		// check if we must send a new clock signal
		// in the case index_pulse has progressed from more than ppq_per_midi_clock ticks
		if (index_pulse > (int) floor (midi_clock_num * ppq_per_midi_clock)) {
			midi_clock_num++;
			send_clock = CLOCK;

/* for debug purpose only
trace_beat++;
sprintf (s2," - CLOCK: %02d", trace_beat);
strcat (s, s2);
*/

		}
		
		// in any case, set previous pulse index to current
		previous_index_pulse = index_pulse;
		
	}

/* for debug purpose only
strcat (s, "\n");
strcpy (trace [trace_index], s);
trace_index = (trace_index > 50000) ? 50000 : trace_index + 1; 
*/

	return FLUID_OK;
}
