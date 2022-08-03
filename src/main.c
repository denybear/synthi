/** @file main.c
 *
 * @brief This is the main file for the looper program. It uses the basic features of JACK.
 *
 */

#include "types.h"
#include "main.h"
#include "config.h"
#include "process.h"


/*************/
/* functions */
/*************/

static void init_globals ( )
{
	int i;

	/******************************/
	/* INIT SOME GLOBAL VARIABLES */
	/******************************/

	/* time signature is 0, ie. 4/4 by default */
	timesign = _4_4;

	/* allocate memory to read parameter table: client/server input-output automated connection */
	/* let's make it 255 strings of 255 characters */
	ports_to_connect = calloc (255, sizeof(char*));
	for (i=0; i<255; i++) ports_to_connect[i] = calloc (255,sizeof(char));

}


static void signal_handler ( int sig )
{
	jack_client_close ( client );
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
	exit ( 1 );
}

/* usage: synthi (config_file) (jack client name) (jack server name)*/

int main ( int argc, char *argv[] )
{
	int i;
	
	// JACK variables
	const char *client_name;
	const char *server_name = NULL;
	char *config_name;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	// Fluidsynth variables
	fluid_settings_t* settings;
	fluid_synth_t* synth;
	fluid_player_t* player;
	fluid_audio_driver_t* adriver;

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

	/* register midi-in port: this port will get the midi keys notification */
	midi_input_port = jack_port_register (client, "midi_input_1", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
	if (midi_input_port == NULL ) {
		fprintf ( stderr, "no more JACK MIDI ports available.\n" );
		exit ( 1 );
	}

	/* register midi-out port: this port will send the midi notifications to light on/off pad leds */
	midi_output_port = jack_port_register (client, "midi_output_1", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
	if (midi_output_port == NULL ) {
		fprintf ( stderr, "no more JACK MIDI ports available.\n" );
		exit ( 1 );
	}


	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

	if ( jack_activate ( client ) )
	{
		fprintf ( stderr, "cannot activate client.\n" );
		exit ( 1 );
	}

	/* init fluidsynth */
	// init fluidsynth
	send_clock = NO_CLOCK;
	settings = new_fluid_settings();
	synth = new_fluid_synth(settings);
	// jack as audio driver
	// sample rate as the one defined in jack
	fluid_settings_setstr(settings, "audio.driver", "jack");
//	fluid_settings_setstr(settings, "synth.sample-rate", sample_rate);
	player = new_fluid_player(synth);
	// start the synthesizer thread
	adriver = new_fluid_audio_driver(settings, synth);
	// set player callback at tick
	fluid_player_set_tick_callback (player, handle_tick, (void *) player);

	/* init global variables */
	init_globals();


	/**************/
	/* MAIN START */
	/**************/

	/* read config file to get all the parameters */
	if (read_config (config_name)==EXIT_FAILURE) {
		fprintf ( stderr, "error in reading config file.\n" );
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


	/* keep running until the transport stops */
	while (1)
	{

		// load soundfont and midi file
		if (fluid_is_soundfont("/usr/share/sounds/sf2/FluidR3_GM.sf2")) {
			fluid_synth_sfload(synth, "/usr/share/sounds/sf2/FluidR3_GM.sf2", 1);
		}
		if (fluid_is_midifile("./ff7.mid")) {
			fluid_player_add(player, "./ff7.mid");
			printf ("file loaded\n");
		}

		// set endless looping of current file
		fluid_player_set_loop (player, -1);

		// play the midi files, if any
		fluid_player_play(player);

		// wait for playback termination
		fluid_player_join(player);

#ifdef WIN32
		Sleep ( 1000 );
#else
		sleep ( 1 );
#endif
	}

	// JACK client close
	jack_client_close ( client );

	// Fluidsynth cleanup
	delete_fluid_audio_driver(adriver);
	delete_fluid_player(player);
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);

	exit ( 0 );
}
