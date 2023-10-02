#ifndef HANDLE_PARAMS_H
#define HANDLE_PARAMS_H

#include <gtk/gtk.h>

/* Parses program arguements in terminal and filters out -c and --config and their values in argv.
 * PARAMATERS: 
 *	   int argc & char ** argv: argc & argv from main. 
 *     int argcFinal & char ** argvFinal: arrays that filtered contents of argv and the new number of arguments are stored in.
 *     char * filename: filename buffer declared in main.
 * RETURN:
 *     return 0 if -c/--config was found, 1 if they were not, -1 if the options were found but the dirs could not be accessed.
 */
int parseArgs(int argc, char ** argv, int * argcFinal, char ** argvFinal, char * filename);

/*Checks if filename can be accessed. 
 * PARAMATER:
 * 	char * filename: Path to check if file exists. 
 * RETURN:
 * 	0: File exists and can be accessed.
 * 	1: File doesn't exist.
 * 	-1: File exists, but cannot be read due to a permissions error.
 * 	-2: File exists, but cannot be written to due to a permissions error.
 * 	-3: File exists, but cannot be read from or written to due to a permissions error.
 */
int attemptAccess(char * filename);

#endif
