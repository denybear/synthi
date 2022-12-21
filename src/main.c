/** @file main.c
 *
 * @brief This is the main file for the looper program. It uses the basic features of JACK.
 *
 */

#include "types.h"
#include "main.h"
#include "config.h"
#include "process.h"
#include "utils.h"
#include "led.h"


/*************/
/* functions */
/*************/
// do not use as you need to have root priviledges for it to work
static int init_gpio_do_not_use ()
{
	if (gpioInitialise() == PI_INIT_FAILED) {
    	fprintf(stderr, "pigpio initialisation failed\n");
    	return OFF;
	}

	/* Set GPIO modes */
	gpioSetMode(LED_GPIO, PI_OUTPUT);
	gpioWrite (LED_GPIO, OFF);					// at start, LED is off

	gpioSetMode(SWITCH_GPIO, PI_INPUT);
	gpioSetPullUpDown(SWITCH_GPIO, PI_PUD_UP);	// Sets a pull-up
	// benefits of pull-up is that way, no voltage are input in the pins; pins are only put to GND
	return ON;
}

// do not use as you need to have root priviledges for it to work
static int kill_gpio_do_not_use ()
{
	gpioTerminate ();
}

static int init_gpio ()
{
	gpio_deamon = pigpio_start(0,0);		// connect to localhost on port 8888

	if (gpio_deamon < 0) {
    	fprintf(stderr, "pigpio initialisation failed\n");
    	return OFF;
	}

	/* Set GPIO modes */
	set_mode (gpio_deamon, LED_GPIO, PI_OUTPUT);
	gpio_write (gpio_deamon, LED_GPIO, OFF);					// at start, LED is off

	set_mode (gpio_deamon, SWITCH_GPIO, PI_INPUT);
	set_pull_up_down (gpio_deamon, SWITCH_GPIO, PI_PUD_UP);	// Sets a pull-up
	// benefits of pull-up is that way, no voltage are input in the pins; pins are only put to GND
	return ON;
}


static int kill_gpio ()
{
	gpio_state = OFF;
	pigpio_stop (gpio_deamon);
}


static void init_globals ( )
{
	int i;

	/******************************/
	/* INIT SOME GLOBAL VARIABLES */
	/******************************/

	/* allocate memory to read parameter table: client/server input-output automated connection */
	/* let's make it 255 strings of 255 characters */
	ports_to_connect = calloc (255, sizeof(char*));
	for (i=0; i<255; i++) ports_to_connect[i] = calloc (255,sizeof(char));

	/* clear structure that will get midi file and SF2 file name details, ie. filename structure */
	for (i = 0; i<NB_NAMES; i++) {
		memset (&filename[i], 0, sizeof (filename_t));
	}
	
	/* clear structure that will get control details for midi file */
	for (i = 0; i<NB_FCT; i++) {
		memset (&filefunct[i], 0, sizeof (filefunct_t));
	}
	
	// init clock sending
	send_clock = NO_CLOCK;

	// clear load/play flags
	// is_load = TRUE allows to load default files (00_*) at startup
	is_load = TRUE;
	is_play = FALSE;
	sf2_id = 0;			// set arbitrary value for sf2_id (current loaded soundfile id)
	
	// function flags
	volume = 2;
	is_volume = TRUE;	// force setting the volume at startup
	bpm = 0	;			// bpm is only set when file is playing
	initial_bpm = -1;
	now = 0;			// used for automated tempo adjustment (at press of switch)
	previous = 0;
	previous_led = 0;	// time when LED was turned ON
}


static void signal_handler ( int sig )
{
	jack_client_close ( client );
	kill_gpio ();
	fprintf ( stderr, "signal received, exiting ...\n" );
	exit ( 0 );
}


/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown ( void *arg )
{
	free (midi_input_port);
	free (midi_output_port);
	free (clock_output_port);
	kill_gpio ();
	exit ( 1 );
}

/* usage: synthi (config_file) (jack client name) (jack server name)*/

int main ( int argc, char *argv[] )
{
	int i,j;
	
	// JACK variables
	const char *client_name;
	const char *server_name = NULL;
	char *config_name;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	// Fluidsynth variables
	fluid_settings_t* settings;
	fluid_synth_t* synth;
	// note that fluid_player_t* player; is declared as global variable as we need it in jack's process () thread
	fluid_audio_driver_t* adriver;
	
	// string containing : directory + filename
	char name [1000];


	/* use basename of argv[0] */
	client_name = strrchr ( argv[0], '/' );
	if ( client_name == 0 ) client_name = argv[0];
	else client_name++;

	/* assign default name to boocli config file */
	config_name = calloc (20, sizeof(char));
	strcpy (config_name, "./synthi.cfg");

	if ( argc >=2 ) {
		free (config_name);
		config_name = argv [1];	// config file name specified
	}

	if ( argc >= 3 )        /* client name specified? */
	{
		client_name = argv[2];
		if ( argc >= 4 )    /* server name specified? */
		{
			server_name = argv[3];
			options |= JackServerName;
		}
	}

	/* open a client connection to the JACK server */

	client = jack_client_open ( client_name, options, &status, server_name );
	if ( client == NULL )
	{
		fprintf ( stderr, "jack_client_open() failed, status = 0x%2.0x.\n", status );
		if ( status & JackServerFailed )
		{
			fprintf ( stderr, "Unable to connect to JACK server.\n" );
		}
		kill_gpio ();
		exit ( 1 );
	}
	if ( status & JackServerStarted )
	{
		fprintf ( stderr, "JACK server started.\n" );
	}
	if ( status & JackNameNotUnique )
	{
		client_name = jack_get_client_name ( client );
		fprintf ( stderr, "unique name `%s' assigned.\n", client_name );
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	/* assign number of frames per packet and sample rate */
	sample_rate = jack_get_sample_rate(client);
	nb_frames_per_packet =  jack_get_buffer_size(client);

	/* set callback function to process jack events */
	jack_set_process_callback ( client, process, 0 );

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/
	jack_on_shutdown ( client, jack_shutdown, 0 );

	/* register midi-in port: this port will get the midi keys notification (from UI) */
	midi_input_port = jack_port_register (client, "midi_input_1", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	if (midi_input_port == NULL ) {
		fprintf ( stderr, "no more JACK MIDI ports available.\n" );
		kill_gpio ();
		exit ( 1 );
	}

	/* register midi-out port: this port will send the midi notifications to light on/off pad leds */
	midi_output_port = jack_port_register (client, "midi_output_1", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
	if (midi_output_port == NULL ) {
		fprintf ( stderr, "no more JACK MIDI ports available.\n" );
		kill_gpio ();
		exit ( 1 );
	}

	/* register clock-out port: this port will send the clock notifications to exteral system (eg. boocli) */
	clock_output_port = jack_port_register (client, "clock_output_1", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
	if (clock_output_port == NULL ) {
		fprintf ( stderr, "no more JACK CLOCK ports available.\n" );
		kill_gpio ();
		exit ( 1 );
	}


	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

	if ( jack_activate ( client ) )
	{
		fprintf ( stderr, "cannot activate client.\n" );
		kill_gpio ();
		exit ( 1 );
	}

	// init GPIO to enable external "beat" switch
	gpio_state = init_gpio ();

	// init global variables
	init_globals();

	// init fluidsynth
	settings = new_fluid_settings();
	synth = new_fluid_synth(settings);
	// jack as audio driver
	// sample rate as the one defined in jack
	fluid_settings_setstr(settings, "audio.driver", "jack");
//	fluid_settings_setstr(settings, "synth.sample-rate", sample_rate);

	// start the synthesizer thread
	adriver = new_fluid_audio_driver(settings, synth);

	// load default soundfont
	if (fluid_is_soundfont(DEFAULT_SF2)) {
		fluid_synth_sfload(synth, DEFAULT_SF2, 1);
	}

	// create new player, but don't load anything for now
	player = new_fluid_player(synth);



	/**************/
	/* MAIN START */
	/**************/

	/* read config file to get all the parameters */
	if (read_config (config_name)==EXIT_FAILURE) {
		fprintf ( stderr, "error in reading config file.\n" );
		kill_gpio ();
		exit ( 1 );
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

	/* go through the list of ports to be connected and connect them by pair (server, client) */
	i=0;
	fprintf (stderr, "attempt to connect input-output ports together.\n");

	while ((ports_to_connect[i][0]!='\x0') && (ports_to_connect[i+1][0]!='\x0')) {
		fprintf (stderr, "server: %s , client: %s\n", ports_to_connect[i], ports_to_connect[i+1]);
		if ( jack_connect ( client, ports_to_connect [i], ports_to_connect [i+1]) ) {
			fprintf ( stderr, "cannot connect ports (between client and server).\n" );
// removed the below 2 lines to so synthi is run in standalone without connection to boocli
//			kill_gpio ();
//			exit (1);
		}
		/* increment index of 2 position to move to next (server, client) port couple */
		i+=2;
	}

	/* clear memory allocated to connect client/server input-output */
	for (i=0; i<255; i++) free (ports_to_connect[i]);
	free (ports_to_connect);



	/* install a signal handler to properly quits jack client */
#ifdef WIN32
	signal ( SIGINT, signal_handler );
	signal ( SIGABRT, signal_handler );
	signal ( SIGTERM, signal_handler );
#else
	signal ( SIGQUIT, signal_handler );
	signal ( SIGTERM, signal_handler );
	signal ( SIGHUP, signal_handler );
	signal ( SIGINT, signal_handler );
#endif

	/* switch all leds off for all filenames */
	for (i = 0; i<NB_NAMES; i++) {
		/* set structure that contains status for each pad led to ON : this is to force all leds off */
		memset (&led_status_filename[i][0], ON, LAST_ELT);
		filename_led_off (i);

		/* set structure that contains status for each pad led to OFF : this is useless: done by led() function, but you never know */
		memset (&led_status_filename[i][0], OFF, LAST_ELT);
	}

	/* switch all leds off for all functions */
	for (i = 0; i<NB_FCT; i++) {
		/* set structure that contains status for each pad led to ON : this is to force all leds off */
		memset (&led_status_filefunct[i][0], ON, LAST_ELT_FCT);
		filefunct_led_off (i);

		/* set structure that contains status for each pad led to OFF : this is useless: done by led() function, but you never know */
		memset (&led_status_filefunct[i][0], OFF, LAST_ELT_FCT);
	}

	// at start, we use default volume (2); light on the volume pads to indicate this to the user
	led_filefunct (0, VOLDOWN, PENDING);
	led_filefunct (0, VOLUP, PENDING);


	/* keep running until the transport stops */
	while (1)
	{
		// check if user has loaded the LOAD button to load midi and SF2 file
		if (is_load) {


/* for debug purpose only
FILE *f;
if (name_to_byte (&filename [0]) == 01) {
	f = fopen ("trace.txt", "wt");
	for (i=0; i<trace_index; i++) {
		fputs(trace[i], f);
	}
	fclose (f);
}
*/
			
			// make sure no file is playing to allow load of new file !
			if ((fluid_player_get_status (player)== FLUID_PLAYER_DONE) || (fluid_player_get_status (player)== FLUID_PLAYER_READY)) {
			
				// get name of requested midi file from directory
				if (get_full_filename (name, name_to_byte (&filename [0]), "./songs/") == TRUE) {
					// if a file exists
					if (fluid_is_midifile(name)) {
						
						// delete current fluid player
						delete_fluid_player (player);
						// create new player
						player = new_fluid_player(synth);
						// set player callback at tick
						fluid_player_set_tick_callback (player, handle_tick, (void *) player);

						// get new ppq value
						ppq = get_division (name); 
						// load midi file
						fluid_player_add(player, name);
						// set endless looping of current file
						fluid_player_set_loop (player, -1);

						// initial bpm of the file is set to -1 to force reading of initial bpm if bpm pads are pressed
						initial_bpm = -1;
						now = 0;			// used for automated tempo adjustment (at press of switch)
						previous = 0;

						// we are at initial BPM; set the 2 BPM pads accordingly
						led_filefunct (0, BPMDOWN, PENDING);
						led_filefunct (0, BPMUP, PENDING);

					}
				}

				// get name of requested SF2 file from directory
				if (get_full_filename (name, name_to_byte (&filename [1]), "./soundfonts/") == TRUE) {
					// if a file exists
					if (fluid_is_soundfont(name)) {
						// unload previously loaded soundfont
						// this is to prevent memory issues (lack of memory)
						fluid_synth_sfunload (synth, sf2_id, TRUE);
						// load new sf2 file
						sf2_id = fluid_synth_sfload(synth, name, TRUE);
					}
				}
			}
			
			// load is done; set to FALSE
			is_load = FALSE;
			// load led OFF
			led_filename (0, LOAD, is_load);
		}

		// check if user has pressed volume pads; this part is also executed at startup
		if (is_volume) {
			// set gain: 0 < gain < 1.0 (default = 0.2)
			fluid_settings_setnum (settings, "synth.gain", (float) volume/10.0f);
			// volume setting is done; set to FALSE
			is_volume = FALSE;
			// no need to light on/off the pads, this is done in the process thread
		}


#ifdef WIN32
		Sleep ( 1000 );
#else
		sleep ( 1 );
#endif
	}

	// terminate gpio support
	kill_gpio ();

	// JACK client close
	jack_client_close ( client );

	// Fluidsynth cleanup
	delete_fluid_audio_driver(adriver);
	delete_fluid_player(player);
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);

	exit ( 0 );
}
