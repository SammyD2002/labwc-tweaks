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

/*Create "AppOptions Struct" to use in g_application_init_cmd_paramaters.*/
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
	if (!file) {
          fprintf(stderr, "warn: fopen(%s) failed\n", filename);
          return;
	}
	fwrite("\n", sizeof("\n"), 1, file);
	fclose(file);
}

int
main(int argc, char **argv)
{	
	char * argvFinal[argc + 1]; /*argc + 1 is used for array size as argv will always include the program name even if argc = 0.*/
	for(int i = 0; i< argc + 1; i++){
		argvFinal[i] = NULL;
	}
	int argcFinal = 0;
	struct state state = { 0 };
	/* Create buffer for filename */
	char filename[4096];
	
	/*Set Config Directory */
	/*Searches array for -c/--config, and sets configSet to the return type.*/
	int configSet = parseArgs(argc, argv, &argcFinal, argvFinal, filename);
	
	if(configSet == 1){	/* If configSet = 1, set config directory to the default location.*/
		char *home = getenv("HOME");
		snprintf(filename, sizeof(filename), "%s/%s", home, ".config/labwc");
	}
	setOUTDIR(filename); //Sets output directory.

	/* read/create config file */
	
	/* Sets filename to the path to rc.xml */
	snprintf(filename, sizeof(filename), "%s/%s", getOUTDIR(), "rc.xml");
	//printf("%s", filename); /*Test statement.*/
	/* Confirms that rc.xml exists, and that it can be accessed if it does. */
	int fileAccess = attemptAccess(filename);
	if(fileAccess == 1){	
		create_basic_rcxml(filename);
	}
	else if(fileAccess == -1){
		printf("%s", "ERROR: rc.xml was found, but cannot be written to. Do you have permission to write to the file?");
		exit(1);
	}
	else if(fileAccess == -2){
		printf("%s", "ERROR: rc.xml was found, but could not be read. Do you have permission to read the file?");
		exit(2);
	}
	else if(fileAccess == -3){
		printf("%s", "ERROR: rc.xml was found, but could not be read from or written to. Do you have permissions to access the file?\n");
		exit(3);
	}
	//printf("%s", filename); /*Test statement.*/
	/* Initialize XML Nodes */
	/* Ensure all relevant nodes exist before we start getting/setting. */
	xml_init(filename);
	xpath_add_node("/labwc_config/theme/cornerRadius");
    xpath_add_node("/labwc_config/theme/name");
    xpath_add_node("/labwc_config/libinput/device/naturalScroll");
    xml_save();

	/*Confirms that environment file exists and can be accessed.*/
	snprintf(filename, sizeof(filename), "%s/%s", getOUTDIR(), "environment");
	fileAccess = attemptAccess(filename);
	if(fileAccess == 1)
		create_env_file(filename);
	else if(fileAccess == -1){
		printf("%s", "ERROR: environment file was found, but cannot be written to. Do you have permission to write to the file?");
		exit(1);
	}
	else if(fileAccess == -2){
		printf("%s", "ERROR: environment file was found, but could not be read. Do you have permission to read the file?");
		exit(2);
	}
	else if(fileAccess == -3){
		printf("%s", "ERROR: environment file was found, but could not be read from or written to. Do you have permissions to access the file?\n");
		exit(3);
	}
	
	/* UI Initialization */
	/* connect to gsettings */
	state.settings = g_settings_new("org.gnome.desktop.interface");
	
	/* start ui */
	GtkApplication *app;
	int status;
	app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), &state);
	g_application_init_cmd_parameters(G_APPLICATION(app), &app_options);
	status = g_application_run(G_APPLICATION(app), argcFinal, argvFinal); /*Pass filtered argv & argc in place of their originals.*/
	g_object_unref(app);

	/* clean up */
	xml_finish();
	pango_cairo_font_map_set_default(NULL);

	return status;
}
