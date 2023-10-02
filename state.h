/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef STATE_H
#define STATE_H
#include <gtk/gtk.h>

/*
* Takes a pointer to an array of characters, then checks the string's length with strlen.
* If strlen resolves successfully copys memory the string is contained in to another array of characters defined in state.c.
*	- strlen should crash if the cstring is not null-terminated.
* Returns outDIR which can be recieved by another function, or ignored.
*/
char* setOUTDIR(char * out);

//Returns pointer to outDIR.
char* getOUTDIR(void);

struct state {
	GtkWidget *window;	
	struct {
		GtkWidget *corner_radius;
		GtkWidget *openbox_theme_name;
		GtkWidget *gtk_theme_name;
		GtkWidget *icon_theme_name;
		GtkWidget *cursor_theme_name;
		GtkWidget *cursor_size;
		GtkWidget *natural_scroll;
		GtkWidget *keyboard_layout;
	} widgets;
	GSettings *settings;
};

//AppOptions struct used to add -c/--config to help menu in main() via g_application_init_cmd_parameters.
typedef struct {
  GString configDir;
} AppOptions;

//Adds the extra options to the help menu as mentioned above.
void g_application_init_cmd_parameters(GApplication *app, AppOptions *options);



#endif /* STATE_H */
