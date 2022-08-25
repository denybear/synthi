/** @file utils.h
 *
 * @brief This file defines prototypes of functions inside utils.c
 *
 */

unsigned char name_to_byte (filename_t *);
int get_full_filename (char *, unsigned char, char *);
int same_event (unsigned char *, unsigned char *);
int push_to_list (int, int , int , int);
int pull_from_list (int *, int *, int *, int *);
