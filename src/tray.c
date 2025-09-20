#include "dbusmenu.h"
#include "ksni-gio.h"
#include "ksni.h"
#include "ksni_host.h"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <string.h>

struct _Tray {
  GObject parent_instance;

  char *icon_name;
  Pixmap *icon_pixmap;

  GDBusConnection *connection;
  const char *unique_name;
  char *alias_name;
  guint owned_id;
  Ksni *ksni;
  KsniHost *ksni_host;
  DBusMenu *dbusmenu;
};

G_DEFINE_TYPE(Tray, tray, G_TYPE_OBJECT)

enum signal_types {
  SIGNAL_CLICK = 0,
  SIGNAL_ITEM_CLICK,
  LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL] = {0};

static void tray_dispose(GObject *object) {
  Tray *tray = TRAY(object);

  if (tray->owned_id > 0) {
    g_bus_unown_name(tray->owned_id);
    tray->owned_id = 0;
  }

  g_clear_pointer(&tray->ksni, g_object_unref);
  g_clear_pointer(&tray->ksni_host, g_object_unref);
  g_clear_pointer(&tray->dbusmenu, g_object_unref);
  g_clear_pointer(&tray->icon_name, g_free);
  g_clear_pointer(&tray->icon_pixmap, g_object_unref);
  g_clear_pointer(&tray->alias_name, g_free);
  g_clear_pointer(&tray->connection, g_object_unref);

  G_OBJECT_CLASS(tray_parent_class)->dispose(object);
}

static void tray_class_init(TrayClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = tray_dispose;
  signals[SIGNAL_CLICK] = g_signal_new_class_handler(
      "click", G_OBJECT_CLASS_TYPE(object_class), G_SIGNAL_RUN_LAST, NULL, NULL,
      NULL, NULL, G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);
  signals[SIGNAL_ITEM_CLICK] = g_signal_new_class_handler(
      "item-click", G_OBJECT_CLASS_TYPE(object_class), G_SIGNAL_RUN_LAST, NULL,
      NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT);
}

static void on_host_appeared(KsniHost *ksni_host, gpointer user_data) {
  Tray *tray = TRAY(user_data);
  g_print("Host appeared\n");

  ksni_host_register(ksni_host, ksni_get_dbus_name(tray->ksni));
}

static void on_ksni_ready(Ksni *ksni, gpointer user_data) {
  (void)ksni;

  Tray *tray = TRAY(user_data);
  g_print("KSNI is ready\n");

  dbusmenu_start(tray->dbusmenu, tray->connection);

  ksni_host_start_watching(tray->ksni_host, tray->connection);
}

static void on_ksni_click(Ksni *ksni, gint x, gint y, gpointer user_data) {
  (void)ksni;
  Tray *tray = TRAY(user_data);
  g_signal_emit(tray, signals[SIGNAL_CLICK], 0, x, y);
}

void on_dbus_connected(GObject *source_object, GAsyncResult *res,
                       gpointer data) {
  (void)source_object;

  GError *error = NULL;
  GDBusConnection *connection = g_bus_get_finish(res, &error);

  if (error != NULL) {
    g_printerr("Failed to connected to session DBus: %s\n", error->message);
    return;
  }

  g_print("DBus connected\n");
  Tray *tray = TRAY(data);
  tray->connection = connection;
  ksni_start(tray->ksni, tray->connection);
}

void forward_item_click(DBusMenu *dbusmenu, guint item_id, gpointer user_data) {
  (void)dbusmenu;
  Tray *tray = TRAY(user_data);
  g_signal_emit(tray, signals[SIGNAL_ITEM_CLICK], 0, item_id);
}

static void tray_init(Tray *tray) {
  tray->ksni = ksni_new();
  g_signal_connect(tray->ksni, "ready", G_CALLBACK(on_ksni_ready), tray);
  g_signal_connect(tray->ksni, "click", G_CALLBACK(on_ksni_click), tray);

  tray->ksni_host = ksni_host_new();
  g_signal_connect(tray->ksni_host, "appeared", G_CALLBACK(on_host_appeared),
                   tray);

  tray->dbusmenu = dbusmenu_new();
  g_signal_connect(tray->dbusmenu, "item-click", G_CALLBACK(forward_item_click),
                   tray);

  g_bus_get(G_BUS_TYPE_SESSION, NULL, on_dbus_connected, tray);
}

Tray *tray_new(void) { return g_object_new(tray_get_type(), NULL); }

void tray_update_title(Tray *tray, const char *title) {
  ksni_update_title(tray->ksni, title);
}

void tray_update_icon_name(Tray *tray, const char *icon_name) {
  ksni_update_icon_name(tray->ksni, icon_name);
}

void tray_update_icon_pixmap(Tray *tray, Pixmap *icon_pixmap) {
  ksni_update_icon_pixmap(tray->ksni, icon_pixmap);
}

void tray_update_tooltip(Tray *tray, const char *tooltip) {
  ksni_update_tooltip(tray->ksni, tooltip);
}

void tray_update_menu(Tray *tray, dbusmenu_item_t *menu) {
  dbusmenu_update(tray->dbusmenu, menu);
}
