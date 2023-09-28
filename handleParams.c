#include "handleParams.h"
//Set up options and their descriptions.
int parseArgs(int argc, char ** argv, int * argcFinal, char ** argvFinal, char * filename){
	int configSet = 1;
	//If argc >= 1, Search Args for -c or --config. When found, attempts to open a file at that location to make sure the dir exists.
	if (argc >= 1){	
		for(int i = 0; i < argc; i++){
			if(strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"--config") == 0){
				int index = i+1;
				int lChar = strlen(argv[index]) - 1;
				if(argv[index][lChar] == '/'){
					argv[index][lChar] = '\0';
				}
				snprintf(filename, 4096, "%s", argv[index]); 
				//increment i to skip config path to avoid conflict with other options.
				i++;
				//set config to 0 to stop later init from running.
				configSet = 0;
			}
			else{
				argvFinal[*argcFinal] = argv[i];
				argcFinal++;
			}
		}
	}
	//If argc = 0, configSet remains at 1.
	return configSet;
}

int attemptAccess(char * filename){
	int exist = access(filename, F_OK);
	if(exist == 0)
		return access(filename, W_OK);
	else
		return 1;
}
