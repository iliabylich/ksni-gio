#include "tray.h"
#include <glib-object.h>
#include <glib.h>

typedef struct {
  GMainLoop *loop;
  Tray *tray;
} exit_data_t;
int do_exit(exit_data_t *exit_data) {
  g_main_loop_quit(exit_data->loop);
  g_object_unref(exit_data->tray);
  return G_SOURCE_REMOVE;
}

int main(void) {
  Tray *tray = tray_new("org.test.TrayTest");

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  exit_data_t exit_data = {.loop = loop, .tray = tray};
  g_timeout_add(5000, G_SOURCE_FUNC(do_exit), &exit_data);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  return 0;
}
