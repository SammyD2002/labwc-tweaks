#include "handleParams.h"
/*Set up extra options and their descriptions in the help (-h) menu.*/
int parseArgs(int argc, char ** argv, int * argcFinal, char ** argvFinal, char * filename){
	int configSet = 1;
	/*If argc >= 1, parses argv, saving all args to argVFinal.*/
	if (argc >= 1){	
		for(int i = 0; i < argc; i++){
			if(strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"--config") == 0){
				int index = i+1;
				int lChar = strlen(argv[index]) - 1; /*Sets lastChar to lastCharacter before '\0'.*/
				if(argv[index][lChar] == '/'){
					argv[index][lChar] = '\0';
				}
				snprintf(filename, 4096, "%s", argv[index]); 
				/*increment i to skip config path to avoid conflict with other options.*/
				i++;
				/*set configSet to 0 to stop the config directory from being set to the default location later.*/
				configSet = 0;
			}
			else{
				argvFinal[*argcFinal] = argv[i];
				argcFinal++;
			}
		}
	}
	else{
		argvFinal[0] = argv[0]; /*If argv < 1, only copies first index with program name that exists regardless of argc.*/
	}
	return configSet; /*If argc = 0, configSet remains at 1.*/
}

/*Return -1 if file could not be read from due to permissions, -2 if it couldn't be written to due to permissions, & -3 for both.*/
int attemptAccess(char * filename){
	int exist = access(filename, F_OK);
	if(exist == 0)
		return (access(filename, R_OK) + (2 * (access(filename, W_OK)))); /*Access returns -1 if the file cannot be accessed.*/
	else
		return 1;
}
