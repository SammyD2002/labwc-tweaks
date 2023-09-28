#include "handleParams.h"
//Set up options and their descriptions.
void
g_application_init_cmd_parameters(GApplication *app, AppOptions *options)
{
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

  g_application_add_main_option_entries (G_APPLICATION (app), cmd_params);
}

