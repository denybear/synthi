/** @file globals.h
 *
 * @brief This file the global variables for the program
 *
 */

// global variables
// define midi ports
extern jack_port_t *midi_input_port;
extern jack_port_t *midi_output_port;
extern jack_port_t *clock_output_port;
extern char **ports_to_connect;

// define JACKD client : this is this program
extern jack_client_t *client;

// number of frames per Jack packet and sample-rate
extern uint32_t nb_frames_per_packet, sample_rate;

// FLUIDSYNTH player
extern fluid_player_t* player;

// determine if midi clock shall be sent or not
extern int send_clock;

// define filename structure for each file name: midi file and SF2 file
extern filename_t filename [];
// define function structure
extern filefunct_t filefunct [];

// define the structures for managing leds of midi control surface
extern unsigned char list_buffer[LIST_ELT][4];	// list buffer of LIST_ELT led request
extern unsigned char list_index;			// index where to write led request to

// status of leds for filenames
extern unsigned char led_status_filename [NB_NAMES][LAST_ELT]; 	// this table will contain whether each light is on/off at a time; this is to avoid sending led requests which are not required
extern unsigned char led_status_filefunct [NB_FCT][LAST_ELT_FCT]; 	// this table will contain whether each light is on/off at a time; this is to avoid sending led requests which are not required

/* load (midi and SF2 files) & play (midi file) globals */
extern int is_load;
extern int is_play;

/* volume and BPM */
extern int bpm;
extern int initial_bpm;
extern int volume;
extern int is_volume;

/* for debug purpose only
extern char trace[50000][80];
extern int trace_index;
extern int trace_beat;
*/
