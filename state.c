#include "state.h"
#include <string.h>
//Set up options and their descriptions. Adapted from example at https://wiki.gnome.org/HowDoI/GtkApplication/CommandLine  
void  
g_application_init_cmd_parameters(GApplication *app, AppOptions *options){ 
    const GOptionEntry cmd_params[] =
    {
		{
        .long_name = "config",
        .short_name = 'c',
        .flags = G_OPTION_FLAG_NONE,     // see `GOptionFlags`
        .arg = G_OPTION_ARG_NONE,        // type of option (see `GOptionArg`)
        .arg_data = &(options->configDir),// store data here
        .description = "Treat path as config directory.",
		.arg_description = NULL,
		},
	{NULL}
	};	
	g_application_add_main_option_entries (G_APPLICATION (app), cmd_params); /*Inserts the options into app.*/
}

/*Config directory stored here by setOUTDIR*/
char OUTDIR[4096];

char* setOUTDIR(char * out){
	/*Get size of filename string's memory*/
	size_t size = strlen(out);
	if(size > 4096){
		size = sizeof(char) * 4095;
	}	
	memcpy(&OUTDIR, out, size); /*Copys out to OUTDIR.*/
	OUTDIR[4095] = '\0';
	return OUTDIR; /*Returns pointer to outdir that may or may not be used.*/
}
  
char* getOUTDIR(void){
	return OUTDIR; /*Returns pointer to OUTDIR.*/
}

