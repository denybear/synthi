/** @file main.h
 *
 * @brief This file contains global variables definition
 *
 */


/********************/
/* global variables */
/********************/

// define if gpio is running for external beat switch and LED
int gpio_state;     // OFF = gpio OFF: ON = GPIO ON
int gpio_deamon;    // deamon id for pigpiod
uint64_t previous_led;  // time when switch was set as on

// define midi ports
jack_port_t *midi_input_port;
jack_port_t *midi_output_port;
jack_port_t *clock_output_port;
char **ports_to_connect;

// define JACKD client : this is this program
jack_client_t *client;

// number of frames per Jack packet and sample-rate
uint32_t nb_frames_per_packet, sample_rate;

// FLUIDSYNTH player
fluid_player_t* player;
fluid_settings_t* settings;
fluid_synth_t* synth;
fluid_audio_driver_t* adriver;

// determine if midi clock shall be sent or not
int send_clock = NO_CLOCK;

// define filename structure for each file name: midi file and SF2 file
filename_t filename [NB_NAMES];
// define function structure
filefunct_t filefunct [NB_FCT];

// define the structures for managing leds of midi control surface
unsigned char list_buffer[LIST_ELT][4];	// list buffer of LIST_ELT led request
unsigned char list_index;			// index where to write led request to

// status of leds for filenames
unsigned char led_status_filename [NB_NAMES][LAST_ELT]; 	// this table will contain whether each light is on/off at a time; this is to avoid sending led requests which are not required
unsigned char led_status_filefunct [NB_FCT][LAST_ELT_FCT]; 	// this table will contain whether each light is on/off at a time; this is to avoid sending led requests which are not required

/* load (midi and SF2 files) & play (midi file) globals */
int is_load;
int is_play;
int sf2_id;		// id of sf2 file currently loaded

/* volume and BPM */
int bpm;
int initial_bpm;
int volume;
int is_volume;

/* PPQ */
int ppq;

/* beat */
uint64_t now;       // time now
uint64_t previous;  // time when "beat" key was last pressed

/* for debug purpose only
char trace[50000][80];
int trace_index = 0;
int trace_beat;
*/

