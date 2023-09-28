#ifndef HANDLE_PARAMS_H
#define HANDLE_PARAMS_H

#include <gtk/gtk.h>

/* Parses program arguements in terminal and filters out -c and --config and their values in argv.
 * PARAMATERS: 
 *	   argc & argv: argc & argv from main. 
 *     argcFinal & argvFinal: arrays that filtered contents of argv and the new number of arguments are stored in.
 *     filename: filename buffer declared in main.
 * RETURN:
 *     return 0 if -c/--config was found, 1 if they were not, -1 if the options were found but the dirs could not be accessed.
 */
int parseArgs(int argc, char ** argv, int * argcFinal, char ** argvFinal, char * filename);

//Checks if filename can be accessed. Returns 1 if it doesn't exist, 0 if it can be written to, and -1 if it can't.
int attemptAccess(char * filename);

#endif
