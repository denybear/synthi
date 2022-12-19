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

// convert filename from structure to a byte 
unsigned char name_to_byte (filename_t *name) {

	return ((name->status[B7] << 7) + (name->status[B6] << 6) + (name->status[B5] << 5) + (name->status[B4] << 4) + (name->status[B3] << 3) + (name->status[B2] << 2) + (name->status[B1] << 1) + (name->status[B0]));
	
}

// in the given directory, look for filename starting with number, and return corresponding full name
// returns FALSE if no file found, TRUE if file is found 
int get_full_filename (char * name, unsigned char number, char * directory) {

	DIR *dir;
	struct dirent *ent;
	char st1[3], st2[3];

	// convert number into string (in lowercases and uppercases)
	sprintf (st1, "%02x", number);
	sprintf (st2, "%02X", number);
	if ((dir = opendir (directory)) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			// check if file starts with number; allow mixing lower and upper cases
			if (((st1[0] == ent->d_name [0]) || (st2[0] == ent->d_name [0])) && ((st1[1] == ent->d_name [1]) || (st2[1] == ent->d_name [1]))) {
				// copy full file name (including directory) into result variable
				strcpy (name, directory);
				strcat (name, ent->d_name);
				closedir (dir);
				return TRUE;
			}
		}
	// could not find a file starting with number
	closedir (dir);
	return FALSE;
	}

	else {
		// could not open directory
		fprintf ( stderr, "Directory not found.\n" );
		return FALSE;
	}
}

// read PPQ from midi file 
int get_division (char * name) {

	FILE *f;
    char mthd[14];
    int division;
    int retour;

	if ((f = fopen (name, "r")) == NULL) printf ("could not open file\n");

	// read file header, in which there is division value
    if (fread (mthd, sizeof(mthd), 1, f) != 1)
    {
		// set division to standard midi clock in case of file read error
        division = 24;
    }

	// else get division (=ppq) value from file 
	else division = ((unsigned)mthd[12] << 8) | ((unsigned)mthd[13] & 0xff);

	fclose (f);
	return division;	
}

// determines if 2 midi messages (ie. events) are the same; returns TRUE if yes
int same_event (unsigned char * evt1, unsigned char * evt2) {

	if ((evt1[0]==evt2[0]) && (evt1[1]==evt2[1])) return TRUE;
	return FALSE;
}


// add led request to the list of requests to be processed
int push_to_list (int dest, int row, int col, int on_off) {

	// add to list
	list_buffer [list_index][0] = (unsigned char) dest;
	list_buffer [list_index][1] = (unsigned char) row;
	list_buffer [list_index][2] = (unsigned char) col;
	list_buffer [list_index][3] = (unsigned char) on_off;

	// increment index and check boundaries
	//list_index = (list_index >= LIST_ELT) ? 0; list_index++;
	if (list_index >= LIST_ELT) {
		fprintf ( stderr, "too many led requests in the list.\n" );
		list_index = 0;
	}
	else list_index++;
}


// pull out led request from the list of requests to be processed (FIFO style)
// returns 0 if pull request has failed (nomore request to be pulled out)
int pull_from_list (int *dest, int *row, int *col, int *on_off) {

	// check if we have requests to be pulled; if not, leave
	if (list_index == 0) return 0;

	// remove first element from list
	*dest = list_buffer [0][0];
	*row = list_buffer [0][1];
	*col = list_buffer [0][2];
	*on_off = list_buffer [0][3];

	// decrement index
	list_index--;

	// move the rest of the list 1 item backwards
	memmove (&list_buffer[0][0], &list_buffer[1][0], list_index * 4);

	return 1;
}



/// Convert seconds to microseconds
#define SEC_TO_US(sec) ((sec)*1000000)
/// Convert nanoseconds to microseconds
#define NS_TO_US(ns)    ((ns)/1000)

/// Get a time stamp in microseconds.
uint64_t micros() {
	
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t us = SEC_TO_US((uint64_t)ts.tv_sec) + NS_TO_US((uint64_t)ts.tv_nsec);
    return us;
}
