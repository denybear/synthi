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

	/* audio outputs */
	setting = config_lookup(&cfg, "connections.output");
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
			/* for outputs, jack port is the output (destination) and shall be second in the array */
			/* "server" should be the actual looper client */
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

	/* successful reading, exit */
	config_destroy(&cfg);
	return(EXIT_SUCCESS);
}
