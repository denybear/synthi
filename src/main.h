/** @file main.h
 *
 * @brief This file contains global variables definition
 *
 */


/********************/
/* global variables */
/********************/

/* define midi ports */
jack_port_t *midi_input_port;
jack_port_t *midi_output_port;
char **ports_to_connect;

/* define JACKD client : this is this program */
jack_client_t *client;

/* number of frames per Jack packet and sample-rate */
uint32_t nb_frames_per_packet, sample_rate;

/* BBT information */
int BBT_numerator=4;        // 4/4 bar time signature at start
int BBT_denominator=4;
unsigned int BBT_bar=1;		// set bar number as 1
int BBT_beat;
int BBT_beat_value;
int	BBT_previous_beat;
int BBT_tick;
int BBT_wait_4_ticks = 8;		// purpose of this is to delay switch on/off of led of about 4 clock ticks, so hardware can support it
int is_BBT=PENDING_ON; 	// OFF: no new bar, ON: new bar, PENDING_ON: (play event, change in sign time) means new bar 1 at the next clock event

int timesign;			// the value of this variable indicates which is the current time signature

int send_clock = NO_CLOCK;

