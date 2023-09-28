// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include "state.h"
#include "stack-appearance.h"
#include "stack-lang.h"
#include "stack-mouse.h"
#include "xml.h"
#include "handleParams.h"

//Create "AppOptions Struct".
static AppOptions app_options;

static void
activate(GtkApplication *app, gpointer user_data)
{
	struct state *state = (struct state *)user_data;

	/* window */
	state->window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(state->window), "Tweaks");
	gtk_window_set_default_size(GTK_WINDOW(state->window), 640, 480);

	/* grid */
	GtkWidget *grid = gtk_grid_new();
	gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
	gtk_container_add(GTK_CONTAINER(state->window), grid);

	/* sidebar + stack */
	GtkWidget *sidebar = gtk_stack_sidebar_new();
	GtkWidget *stack = gtk_stack_new();
	GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	gtk_grid_attach(GTK_GRID(grid), sidebar, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), separator, 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), stack, 2, 0, 1, 1);
	gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(sidebar), GTK_STACK(stack));

	/* Stacks */
	stack_appearance_init(state, stack);
	stack_mouse_init(state, stack);
	stack_lang_init(state, stack);

	gtk_widget_show_all(state->window);
}


static const char rcxml_template[] =
	"<?xml version=\"1.0\"?>\n"
	"<labwc_config>\n"
	"  <core>\n"
	"  </core>\n"
	"</labwc_config>\n";

static void
create_basic_rcxml(const char *filename)
{
	FILE *file = fopen(filename, "w");
	if (!file) {
		fprintf(stderr, "warn: fopen(%s) failed\n", filename);
		return;
	}
	if (!fwrite(rcxml_template, sizeof(rcxml_template), 1, file)) {
		fprintf(stderr, "warn: error writing to %s", filename);
	}
	fclose(file);
}

static void
create_env_file(const char *filename){
	FILE *file = fopen(filename, "w");
	if(!file) if (!file) {
          fprintf(stderr, "warn: fopen(%s) failed\n", filename);
          return;
	}
	fwrite("\n", sizeof("\n"), 1, file);
	fclose(file);
}

int
main(int argc, char **argv)
{	/*if(1 == 1){
		printf("%s", "DEBUG STATEMENTS\nARGC: ");
		printf("%i", argc);
		printf("%s", "\nARGV Elements:");
		for(int i = 0; i < argc; i++){
			printf("%s\n", argv[i]);
		}
		printf("%s", "Exiting...");
		exit(1);
	}*/
	char * argvFinal[argc + 1]; //argc + 1 accounts for the fact argv contains program name even with 0 args.
	for(int i = 0; i< argc + 1; i++){
		argvFinal[i] = NULL;
	}
	int argcFinal = 0;
	struct state state = { 0 };
	/* Create buffer for filename */
	char filename[4096];
	/* read/create config file */
	int configSet = 1;
	if (argc >= 1){ /*If argc >= 1, Search Args for -c or --config. When found, attempts to open a file at that location to make sure the dir exists.*/
		for(int i = 0; i < argc; i++){
			if(strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"--config") == 0){
				int index = i+1;
				int lChar = strlen(argv[index]) - 1; //-1 to get to final element, -2 to skip \0.
				if(argv[index][lChar] == '/'){
					argv[index][lChar] = '\0';
				}
				snprintf(filename, sizeof(filename), "%s/%s", argv[index], "rc.xml");
				printf("%s", filename);
				int success = -1;
				if (access(filename, F_OK)) {
              		create_basic_rcxml(filename);
					success = 0;
				}
				else if(access(filename, R_OK) == 0){
					printf("%s", "File found on fs.");
					success = 0;
				}
				else{
					printf("%s","ERROR: File not found.\n");
					exit(1);
				}
				if(success == 0){
					xml_init(filename);
					//Sets up nodes.
					xpath_add_node("/labwc_config/theme/cornerRadius");
      				xpath_add_node("/labwc_config/theme/name");
				    xpath_add_node("/labwc_config/libinput/device/naturalScroll");
					xml_save();
					//Makes sure additional settings files exist.
					snprintf(filename, sizeof(filename), "%s/%s", argv[index], "environment"); 
					if (access(filename, F_OK) != 0 && access(filename, R_OK) != 0){
						printf("Could not open enviornment file.");
						create_env_file(filename);
					}
					//Set filename back to directory.
					snprintf(filename, sizeof(filename), "%s", argv[index]); 
				}
				//increment i to skip
				i++;
				//set config to 0 to stop later init from running.
				configSet = 0;
			}
			else{
				argvFinal[argcFinal] = argv[i];
				argcFinal++;
			}
		}
	}
	if(configSet == 1){	/*Otherwise, attempt to load file from default locations. configSet will be set to 0*/
		char *home = getenv("HOME");
		snprintf(filename, sizeof(filename), "%s/%s", home, ".config/labwc/rc.xml");
		if (access(filename, F_OK)) {
			create_basic_rcxml(filename);
		}
		//printf("%s", filename);
		xml_init(filename);
		xpath_add_node("/labwc_config/theme/cornerRadius");
      	xpath_add_node("/labwc_config/theme/name");
      	xpath_add_node("/labwc_config/libinput/device/naturalScroll");
      	xml_save();

	}

	/* ensure all relevant nodes exist before we start getting/setting */
	
	//printf("%s", filename);
	setOUTDIR(filename);
	//printf("%s", getOUTDIR());
	/* connect to gsettings */
	state.settings = g_settings_new("org.gnome.desktop.interface");
	
	/* start ui */
	GtkApplication *app;
	int status;
	app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), &state);
	g_application_init_cmd_parameters(G_APPLICATION(app), &app_options);
	status = g_application_run(G_APPLICATION(app), argcFinal, argvFinal); //Lie to make app run as expected. (TMP Fix)
	g_object_unref(app);

	/* clean up */
	xml_finish();
	pango_cairo_font_map_set_default(NULL);

	return status;
}
