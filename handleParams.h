#include <gtk/gtk.h>

typedef struct {
  GString configDir;
} AppOptions;

void g_application_init_cmd_parameters(GApplication *app, AppOptions *options);

