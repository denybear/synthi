/** @file globals.h
 *
 * @brief This file the global variables for the program
 *
 */

/* global variables */
/* define midi ports */
extern jack_port_t *midi_input_port;
extern jack_port_t *midi_output_port;
extern char **ports_to_connect;

/* define JACKD client : this is this program */
extern jack_client_t *client;

/* number of frames per Jack packet and sample-rate */
extern uint32_t nb_frames_per_packet, sample_rate;

/* BBT information */
/* warning ! in case of change in time signature, we will need to reset tick_counter as well */
extern int BBT_numerator;
extern int BBT_denominator;
extern unsigned int BBT_bar;
extern int BBT_beat;
extern int BBT_beat_value;
extern int BBT_previous_beat;
extern int BBT_tick;
extern int BBT_wait_4_ticks;
extern int is_BBT; 	// OFF: no new bar, ON: new bar, PENDING_ON: (play event, change in sign time) means new bar 1 at the next clock event

extern int timesign;					// the value of this variable indicates which is the current time signature

extern int send_clock;

