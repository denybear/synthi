/** @file config.c
 *
 * @brief The config module reads looper config file to set key parameters such as:
 * - automated connection of the client to JACK ports
 * - control surface MIDI messages (connected to MIDI in)
 *
 */

#include "types.h"
#include "globals.h"
#include "config.h"
#include "process.h"
#include "utils.h"
#include "led.h"


/* This example reads the configuration file 'example.cfg' and displays
 * some of its contents.
 */

int read_config (char *name)
{
	config_t cfg;
	config_setting_t *setting;
	const char *str;
	int index;
	int i;
	config_setting_t *buffer;


	config_init(&cfg);

	/* Read the file. If there is an error, report it and exit. */
	if(! config_read_file(&cfg,name))
	{
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
		config_destroy(&cfg);
		return(EXIT_FAILURE);
	}

	/* Read a dummy name; this is mostly to remember how to read a single config parameter */
	if(!config_lookup_string(&cfg, "name", &str)) fprintf ( stderr, "Unable to read config name.\n" );

	/****************************************************************************/
	/* Read connection settings : connection of server port X to client port Y  */
	/****************************************************************************/

	index = 0;


	// audio outputs for FLUIDSYNTH
	setting = config_lookup(&cfg, "connections.output");
	if(setting != NULL)
	{
		int count = config_setting_length(setting);

		for(i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem(setting, i);

			// Only output the record if all of the expected fields are present.
			const char *port_server, *port_client;

			if(!(config_setting_lookup_string(book, "server", &port_server)
					 && config_setting_lookup_string(book, "client", &port_client)))
				continue;

			// copy the ports found in config file to an array of string, and increment the index in the table
			// for outputs, jack port is the output (destination) and shall be second in the array
			// "server" should be the actual looper client
			strcpy (ports_to_connect [index++], port_server);
			strcpy (ports_to_connect [index++], port_client);
		}
	}

	/* midi inputs */
	setting = config_lookup(&cfg, "connections.midi_input");
	if(setting != NULL)
	{
		int count = config_setting_length(setting);

		for(i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem(setting, i);

			/* Only output the record if all of the expected fields are present. */
			const char *port_server, *port_client;

			if(!(config_setting_lookup_string(book, "server", &port_server)
					 && config_setting_lookup_string(book, "client", &port_client)))
				continue;

			/* copy the ports found in config file to an array of string, and increment the index in the table */
			/* for inputs, jack port is the input and shall be first in the array */
			strcpy (ports_to_connect [index++], port_server);
			strcpy (ports_to_connect [index++], port_client);
		}
	}

	/* midi outputs */
	setting = config_lookup(&cfg, "connections.midi_output");
	if(setting != NULL)
	{
		int count = config_setting_length(setting);

		for(i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem(setting, i);

			/* Only output the record if all of the expected fields are present. */
			const char *port_server, *port_client;

			if(!(config_setting_lookup_string(book, "server", &port_server)
					 && config_setting_lookup_string(book, "client", &port_client)))
				continue;

			/* copy the ports found in config file to an array of string, and increment the index in the table */
			/* for inputs, jack port is the input and shall be first in the array */
			strcpy (ports_to_connect [index++], port_server);
			strcpy (ports_to_connect [index++], port_client);
		}
	}

	/* clock outputs */
	setting = config_lookup(&cfg, "connections.clock_output");
	if(setting != NULL)
	{
		int count = config_setting_length(setting);

		for(i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem(setting, i);

			/* Only output the record if all of the expected fields are present. */
			const char *port_server, *port_client;

			if(!(config_setting_lookup_string(book, "server", &port_server)
					 && config_setting_lookup_string(book, "client", &port_client)))
				continue;

			/* copy the ports found in config file to an array of string, and increment the index in the table */
			/* for inputs, jack port is the input and shall be first in the array */
			strcpy (ports_to_connect [index++], port_server);
			strcpy (ports_to_connect [index++], port_client);
		}
	}


	/*****************************************************************************************************/
	/* Read filename settings : assign midi events to control filename of midi file and SF2 to be played */
	/*****************************************************************************************************/

	/* control inputs */
	setting = config_lookup(&cfg, "filename.controls");
	if (setting != NULL)
	{
		int count = config_setting_length(setting);

		/* Check we don't have a too large number of tracks defined, in which case we set to the maximum */
		if (count > NB_NAMES) count = NB_NAMES;

		 /* read element by element */
		for (i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem (setting, i);

			/* read member by member */
			/* play pad */
			buffer = config_setting_get_member (book, "play");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[PLAY][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[PLAY][1] = config_setting_get_int_elem (buffer, 1);

			/* load pad */
			buffer = config_setting_get_member (book, "load");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[LOAD][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[LOAD][1] = config_setting_get_int_elem (buffer, 1);

			/* bit */
			buffer = config_setting_get_member (book, "b7");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[B7][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[B7][1] = config_setting_get_int_elem (buffer, 1);

			/* bit */
			buffer = config_setting_get_member (book, "b6");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[B6][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[B6][1] = config_setting_get_int_elem (buffer, 1);

			/* bit */
			buffer = config_setting_get_member (book, "b5");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[B5][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[B5][1] = config_setting_get_int_elem (buffer, 1);

			/* bit */
			buffer = config_setting_get_member (book, "b4");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[B4][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[B4][1] = config_setting_get_int_elem (buffer, 1);

			/* bit */
			buffer = config_setting_get_member (book, "b3");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[B3][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[B3][1] = config_setting_get_int_elem (buffer, 1);

			/* bit */
			buffer = config_setting_get_member (book, "b2");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[B2][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[B2][1] = config_setting_get_int_elem (buffer, 1);

			/* bit */
			buffer = config_setting_get_member (book, "b1");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[B1][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[B1][1] = config_setting_get_int_elem (buffer, 1);

			/* bit */
			buffer = config_setting_get_member (book, "b0");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filename[i].ctrl[B0][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].ctrl[B0][1] = config_setting_get_int_elem (buffer, 1);
		}
	}


	/* led on */
	setting = config_lookup(&cfg, "filename.led_on");
	if (setting != NULL)
	{
		int count = config_setting_length(setting);

		/* Check we don't have a too large number of tracks defined, in which case we set to the maximum */
		if (count > NB_NAMES) count = NB_NAMES;

		 /* read element by element */
		for (i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem (setting, i);

			/* read member by member */
			/* play pad */
			buffer = config_setting_get_member (book, "play");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[PLAY][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[PLAY][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[PLAY][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* load pad */
			buffer = config_setting_get_member (book, "load");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[LOAD][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[LOAD][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[LOAD][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b7");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B7][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B7][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B7][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b6");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B6][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B6][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B6][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b5");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B5][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B5][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B5][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b4");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B4][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B4][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B4][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b3");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B3][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B3][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B3][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b2");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B2][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B2][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B2][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b1");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B1][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B1][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B1][ON][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b0");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B0][ON][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B0][ON][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B0][ON][2] = config_setting_get_int_elem (buffer, 2);
		}
	}


	/* led off */
	setting = config_lookup(&cfg, "filename.led_off");
	if (setting != NULL)
	{
		int count = config_setting_length(setting);

		/* Check we don't have a too large number of tracks defined, in which case we set to the maximum */
		if (count > NB_NAMES) count = NB_NAMES;

		 /* read element by element */
		for (i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem (setting, i);

			/* read member by member */
			/* play pad */
			buffer = config_setting_get_member (book, "play");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[PLAY][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[PLAY][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[PLAY][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* load pad */
			buffer = config_setting_get_member (book, "load");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[LOAD][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[LOAD][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[LOAD][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b7");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B7][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B7][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B7][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b6");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B6][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B6][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B6][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b5");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B5][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B5][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B5][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b4");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B4][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B4][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B4][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b3");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B3][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B3][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B3][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b2");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B2][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B2][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B2][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b1");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B1][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B1][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B1][OFF][2] = config_setting_get_int_elem (buffer, 2);

			/* bit */
			buffer = config_setting_get_member (book, "b0");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filename[i].led[B0][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filename[i].led[B0][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filename[i].led[B0][OFF][2] = config_setting_get_int_elem (buffer, 2);
		}
	}


	/*******************************************************************************/
	/* Read functions settings : assign midi events to control vol -/+ and bpm -/+ */
	/*******************************************************************************/

	/* control inputs */
	setting = config_lookup(&cfg, "functions.controls");
	if (setting != NULL)
	{
		int count = config_setting_length(setting);

		/* Check we don't have a too large number of tracks defined, in which case we set to the maximum */
		if (count > NB_FCT) count = NB_FCT;

		 /* read element by element */
		 /* only 1 element in our case */
		for (i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem (setting, i);

			/* read member by member */
			buffer = config_setting_get_member (book, "voldown");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filefunct[i].ctrl[VOLDOWN][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].ctrl[VOLDOWN][1] = config_setting_get_int_elem (buffer, 1);

			buffer = config_setting_get_member (book, "volup");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filefunct[i].ctrl[VOLUP][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].ctrl[VOLUP][1] = config_setting_get_int_elem (buffer, 1);

			buffer = config_setting_get_member (book, "bpmdown");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filefunct[i].ctrl[BPMDOWN][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].ctrl[BPMDOWN][1] = config_setting_get_int_elem (buffer, 1);

			buffer = config_setting_get_member (book, "bpmup");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filefunct[i].ctrl[BPMUP][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].ctrl[BPMUP][1] = config_setting_get_int_elem (buffer, 1);

			buffer = config_setting_get_member (book, "beat");
			/* check buffer is not empty, and has 2 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=2) continue;
			filefunct[i].ctrl[BEAT][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].ctrl[BEAT][1] = config_setting_get_int_elem (buffer, 1);

		}
	}

	/* led on */
	setting = config_lookup(&cfg, "functions.led_on");
	if (setting != NULL)
	{
		int count = config_setting_length(setting);

		/* Check we don't have a too large number of tracks defined, in which case we set to the maximum */
		if (count > NB_FCT) count = NB_FCT;

		 /* read element by element */
		 /* only 1 element in our case */
		for (i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem (setting, i);

			/* read member by member */
			buffer = config_setting_get_member (book, "voldown");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[VOLDOWN][ON][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[VOLDOWN][ON][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[VOLDOWN][ON][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "volup");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[VOLUP][ON][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[VOLUP][ON][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[VOLUP][ON][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "bpmdown");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BPMDOWN][ON][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BPMDOWN][ON][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BPMDOWN][ON][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "bpmup");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BPMUP][ON][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BPMUP][ON][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BPMUP][ON][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "beat");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BEAT][ON][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BEAT][ON][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BEAT][ON][2] = config_setting_get_int_elem (buffer, 2);
		}
	}


	/* led pending */
	setting = config_lookup(&cfg, "functions.led_pending");
	if (setting != NULL)
	{
		int count = config_setting_length(setting);

		/* Check we don't have a too large number of tracks defined, in which case we set to the maximum */
		if (count > NB_FCT) count = NB_FCT;

		 /* read element by element */
		 /* only 1 element in our case */
		for (i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem (setting, i);

			/* read member by member */
			buffer = config_setting_get_member (book, "voldown");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[VOLDOWN][PENDING][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[VOLDOWN][PENDING][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[VOLDOWN][PENDING][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "volup");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[VOLUP][PENDING][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[VOLUP][PENDING][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[VOLUP][PENDING][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "bpmdown");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BPMDOWN][PENDING][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BPMDOWN][PENDING][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BPMDOWN][PENDING][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "bpmup");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BPMUP][PENDING][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BPMUP][PENDING][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BPMUP][PENDING][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "beat");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BEAT][PENDING][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BEAT][PENDING][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BEAT][PENDING][2] = config_setting_get_int_elem (buffer, 2);
		}
	}


	/* led off */
	setting = config_lookup(&cfg, "functions.led_off");
	if (setting != NULL)
	{
		int count = config_setting_length(setting);

		/* Check we don't have a too large number of tracks defined, in which case we set to the maximum */
		if (count > NB_FCT) count = NB_FCT;

		 /* read element by element */
		 /* only 1 element in our case */
		for (i = 0; i < count; ++i)
		{
			config_setting_t *book = config_setting_get_elem (setting, i);

			/* read member by member */
			buffer = config_setting_get_member (book, "voldown");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[VOLDOWN][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[VOLDOWN][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[VOLDOWN][OFF][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "volup");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[VOLUP][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[VOLUP][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[VOLUP][OFF][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "bpmdown");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BPMDOWN][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BPMDOWN][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BPMDOWN][OFF][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "bpmup");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BPMUP][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BPMUP][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BPMUP][OFF][2] = config_setting_get_int_elem (buffer, 2);

			buffer = config_setting_get_member (book, "beat");
			/* check buffer is not empty, and has 3 elements */
			if (!buffer) continue;
			if (config_setting_length(buffer)!=3) continue;
			filefunct[i].led[BEAT][OFF][0] = config_setting_get_int_elem (buffer, 0);
			filefunct[i].led[BEAT][OFF][1] = config_setting_get_int_elem (buffer, 1);
			filefunct[i].led[BEAT][OFF][2] = config_setting_get_int_elem (buffer, 2);
		}
	}


	/* successful reading, exit */
	config_destroy(&cfg);
	return(EXIT_SUCCESS);
}
