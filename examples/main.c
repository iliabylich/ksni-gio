#include "api.h"
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

int counter = 0;

const char *gen_title(void) {
  if (counter % 2 == 0) {
    return "even title";
  } else {
    return "odd title";
  }
}
const char *gen_icon_name(void) {
  if (counter % 2 == 0) {
    return "edit-copy";
  } else {
    return "edit-delete";
  }
}
gboolean radio_checked = FALSE;
gboolean checkbox_checked = FALSE;
dbusmenu_item_t *gen_menu(void) {
  guint id = 0;
  dbusmenu_item_t *root = dbusmenu_item_new_root(id++);

  const char *label;
  if (counter % 2 == 0) {
    label = "standrd-even";
  } else {
    label = "standard-odd";
  }
  dbusmenu_item_t *standard =
      dbusmenu_item_new_standard(id++, label, TRUE, TRUE);
  dbusmenu_item_submenu_push_child(root, standard);

  dbusmenu_item_t *disabled =
      dbusmenu_item_new_standard(id++, "must be disabled", FALSE, TRUE);
  dbusmenu_item_submenu_push_child(root, disabled);

  dbusmenu_item_t *hidden =
      dbusmenu_item_new_standard(id++, "must be hidden", TRUE, FALSE);
  dbusmenu_item_submenu_push_child(root, hidden);

  dbusmenu_item_t *radio =
      dbusmenu_item_new_radio(id++, "must be radio", TRUE, TRUE, radio_checked);
  dbusmenu_item_submenu_push_child(root, radio);

  dbusmenu_item_t *checkbox = dbusmenu_item_new_checkbox(
      id++, "must be checkbox", TRUE, TRUE, checkbox_checked);
  dbusmenu_item_submenu_push_child(root, checkbox);

  dbusmenu_item_t *submenu =
      dbusmenu_item_new_submenu(id++, "must be submenu", TRUE);
  dbusmenu_item_submenu_push_child(root, submenu);

  dbusmenu_item_t *child1 =
      dbusmenu_item_new_standard(id++, "child 1", TRUE, TRUE);
  dbusmenu_item_submenu_push_child(submenu, child1);

  dbusmenu_item_t *child2 =
      dbusmenu_item_new_standard(id++, "child 2", TRUE, TRUE);
  dbusmenu_item_submenu_push_child(submenu, child2);

  return root;
}

void tray_update_everything(Tray *tray) {
  counter++;
  tray_update_title(tray, gen_title());
  tray_update_icon_name(tray, gen_icon_name());
  tray_update_menu(tray, gen_menu());
}

int every_second(Tray *tray) {
  g_print("tick\n");
  tray_update_everything(tray);
  return G_SOURCE_CONTINUE;
}

void on_tray_click(Tray *tray, int x, int y, gpointer user_data) {
  (void)tray;
  (void)user_data;
  g_print("Tray clicked at x=%d y=%d\n", x, y);
}

int main(void) {
  Tray *tray = tray_new();
  tray_update_everything(tray);
  g_signal_connect(tray, "click", G_CALLBACK(on_tray_click), NULL);

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  // exit_data_t exit_data = {.loop = loop, .tray = tray};
  // g_timeout_add(5000, G_SOURCE_FUNC(do_exit), &exit_data);

  // g_timeout_add(1000, G_SOURCE_FUNC(every_second), tray);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  return 0;
}
