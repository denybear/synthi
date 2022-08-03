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
	jack_midi_event_t in_event;
	jack_midi_data_t buffer[5];				// midi out buffer for lighting the pad leds



	/**************************************/
	/* First, process MIDI in (UI) events */
	/**************************************/

	// allocate structure that will receive MIDI events
	//clock_event = calloc (1, sizeof (jack_midi_event_t));
	//in_event = calloc (1, sizeof (jack_midi_event_t));

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
	midiout = jack_port_get_buffer (midi_output_port, nframes);

	// clear midi write buffer
	jack_midi_clear_buffer (midiout);

	// start of the song
	// check if we should send midi PLAY (+ midi clock right next)
	if (send_clock == CLOCK_PLAY) {
		buffer [0] = MIDI_PLAY;
		jack_midi_event_write (midiout, 0, buffer, 1);
		// send MIDI_CLOCK right after MIDI_PLAY by setting send_clock
		send_clock = CLOCK;
	}
	
	// check if we should send midi clock
	if (send_clock == CLOCK) {
		buffer [0] = MIDI_CLOCK;
		jack_midi_event_write (midiout, 0, buffer, 1);
		// clear send_clock flag; it will be set back again by tick handling process
		send_clock = NO_CLOCK;
	}


	/***************************************/
	/* Third, process MIDI out (UI) events */
	/***************************************/

	return 0;
}


// process callback called to process midi_in events in realtime
int midi_in_process (jack_midi_event_t *event, jack_nframes_t nframes) {

	int i,j;
	int k,l;

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

	// define data as being a pointer to player
	player = (fluid_player_t*) data;
	// pulse per quarter note
	ppq = fluid_player_get_division (player);
	// number of pulse per midi_clock event
	ppq_per_midi_clock = ppq / 24.0;
	// index of pulse within quarter note
	index_pulse = tick % ppq;
	
	// start of track: let's send PLAY
	if (tick == 0) {
		send_clock = CLOCK_PLAY;
		// let's send first MIDI_CLOCK signal of the beat
		midi_clock_num = 1;
		printf ("PPQ: %d\n", ppq);
		// set previous_index_pulse to 0, as this is the start of the song
		// at start of song, we send MIDI_PLAY then MIDI_CLOCK
		// hence by setting to 0, we indicate we have sent MIDI_CLOCK already
		previous_index_pulse = 0;
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
//			printf ("beat\n");
//			printf ("midi_clock_num: %d, index:%d\n",midi_clock_num, index_pulse);
		}
		
		// check if we must send a new clock signal
		// in the case index_pulse has progressed from more than ppq_per_midi_clock ticks
		if (index_pulse > (int) floor (midi_clock_num * ppq_per_midi_clock)) {
				midi_clock_num++;
				send_clock = CLOCK;
//			printf ("midi_clock_num: %d, index:%d\n",midi_clock_num, index_pulse);
		}
		
		// in any case, set previous pulse index to current
		previous_index_pulse = index_pulse;
		
	}
	
	return FLUID_OK;
}
