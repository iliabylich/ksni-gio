#include "ksni-gio.h"
#include <glib-object.h>
#include <glib.h>

gboolean exiting = FALSE;
typedef struct {
  GMainLoop *loop;
  Tray *tray;
} exit_data_t;
int do_exit(exit_data_t *exit_data) {
  exiting = TRUE;
  g_object_unref(exit_data->tray);
  g_main_loop_quit(exit_data->loop);
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
#define ROOT_ID 0
#define STANDARD_ITEM_ID 1
#define DISABLED_ITEM_ID 2
#define HIDDEN_ITEM_ID 3
#define RADIO1_ITEM_ID 4
#define RADIO2_ITEM_ID 5
#define CHECKBOX_ITEM_ID 6
#define SUBMENU_ITEM_ID 7
#define CHILD1_ITEM_ID 8
#define CHILD2_ITEM_ID 9

guint selected_radio_id = RADIO1_ITEM_ID;
gboolean checkbox_checked = FALSE;

dbusmenu_item_t *gen_menu(void) {
  dbusmenu_item_t *root = dbusmenu_item_new_root(ROOT_ID);

  const char *label;
  if (counter % 2 == 0) {
    label = "standrd-even";
  } else {
    label = "standard-odd";
  }
  dbusmenu_item_t *standard =
      dbusmenu_item_new_standard(STANDARD_ITEM_ID, label, TRUE, TRUE);
  dbusmenu_item_submenu_push_child(root, standard);

  dbusmenu_item_t *disabled = dbusmenu_item_new_standard(
      DISABLED_ITEM_ID, "must be disabled", FALSE, TRUE);
  dbusmenu_item_submenu_push_child(root, disabled);

  dbusmenu_item_t *hidden =
      dbusmenu_item_new_standard(HIDDEN_ITEM_ID, "must be hidden", TRUE, FALSE);
  dbusmenu_item_submenu_push_child(root, hidden);

  dbusmenu_item_t *radio1 =
      dbusmenu_item_new_radio(RADIO1_ITEM_ID, "radio1", TRUE, TRUE,
                              selected_radio_id == RADIO1_ITEM_ID);
  dbusmenu_item_submenu_push_child(root, radio1);

  dbusmenu_item_t *radio2 =
      dbusmenu_item_new_radio(RADIO2_ITEM_ID, "radio2", TRUE, TRUE,
                              selected_radio_id == RADIO2_ITEM_ID);
  dbusmenu_item_submenu_push_child(root, radio2);

  dbusmenu_item_t *checkbox = dbusmenu_item_new_checkbox(
      CHECKBOX_ITEM_ID, "must be checkbox", TRUE, TRUE, checkbox_checked);
  dbusmenu_item_submenu_push_child(root, checkbox);

  dbusmenu_item_t *submenu =
      dbusmenu_item_new_submenu(SUBMENU_ITEM_ID, "must be submenu", TRUE);
  dbusmenu_item_submenu_push_child(root, submenu);

  dbusmenu_item_t *child1 =
      dbusmenu_item_new_standard(CHILD1_ITEM_ID, "child 1", TRUE, TRUE);
  dbusmenu_item_submenu_push_child(submenu, child1);

  dbusmenu_item_t *child2 =
      dbusmenu_item_new_standard(CHILD2_ITEM_ID, "child 2", TRUE, TRUE);
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
  if (!exiting) {
    tray_update_everything(tray);
  }
  return G_SOURCE_CONTINUE;
}

void on_tray_click(Tray *tray, int x, int y, gpointer user_data) {
  (void)tray;
  (void)user_data;
  g_print("Tray clicked at x=%d y=%d\n", x, y);
}

void on_tray_item_click(Tray *tray, unsigned int item_id, gpointer user_data) {
  (void)user_data;
  g_print("Tray item clicked id=%u\n", item_id);
  switch (item_id) {
  case RADIO1_ITEM_ID:
    g_print("selecting radio1\n");
    selected_radio_id = RADIO1_ITEM_ID;
    break;
  case RADIO2_ITEM_ID:
    g_print("selecting radio2\n");
    selected_radio_id = RADIO2_ITEM_ID;
    break;

  case CHECKBOX_ITEM_ID:
    g_print("Toggling checkbox\n");
    checkbox_checked = !checkbox_checked;
    break;
  }
  tray_update_menu(tray, gen_menu());
}

int main(void) {
  Tray *tray = tray_new();
  tray_update_everything(tray);
  g_signal_connect(tray, "click", G_CALLBACK(on_tray_click), NULL);
  g_signal_connect(tray, "item-click", G_CALLBACK(on_tray_item_click), NULL);

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);

  exit_data_t exit_data = {.loop = loop, .tray = tray};
  if (getenv("TEST_EXIT") != NULL) {
    g_timeout_add(2000, G_SOURCE_FUNC(do_exit), &exit_data);
  }

  if (getenv("DO_CHANGES_AUTOMATICALLY") != NULL) {
    g_timeout_add(100, G_SOURCE_FUNC(every_second), tray);
  }

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  return 0;
}
