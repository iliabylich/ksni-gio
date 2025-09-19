#include "tray.h"
#include "ksni.h"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <string.h>

struct _Tray {
  GObject parent_instance;

  char *dbus_name;

  GDBusConnection *connection;
  guint owned_id;
  Ksni *ksni;
};

G_DEFINE_TYPE(Tray, tray, G_TYPE_OBJECT)

enum signal_types {
  SIGNAL_ERROR = 0,
  LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL] = {0};

static void tray_dispose(GObject *object) {
  g_print("TRAY dispose...\n");
  Tray *tray = TRAY(object);
  g_bus_unown_name(tray->owned_id);
  g_clear_pointer(&tray->ksni, g_object_unref);
}

static void tray_class_init(TrayClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = tray_dispose;
  signals[SIGNAL_ERROR] = g_signal_new_class_handler(
      "error", G_OBJECT_CLASS_TYPE(object_class), G_SIGNAL_RUN_LAST, NULL, NULL,
      NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
}

void on_bus_acquired(GDBusConnection *connection, const gchar *name,
                     gpointer user_data) {
  (void)name;
  g_print("bus acquired\n");
  Tray *tray = TRAY(user_data);
  tray->connection = connection;
}

void on_name_acquired(GDBusConnection *connection, const gchar *name,
                      gpointer user_data) {
  (void)connection;
  (void)name;
  g_print("name acquired\n");
  Tray *tray = TRAY(user_data);

  ksni_register(tray->ksni, connection);
}

void on_name_lost(GDBusConnection *connection, const gchar *name,
                  gpointer user_data) {
  (void)connection;
  (void)name;
  g_print("name lost\n");
  Tray *tray = TRAY(user_data);
  ksni_unregister(tray->ksni, tray->connection);
}

static void tray_init(Tray *tray) {
  tray->ksni = ksni_new("test-app-id");
  // g_object_set(                                                      //
  //     tray->ksni,                                                    //
  //     "Title", "test-title",                                         //
  //     "IconPixmap", pixmap_new(3, 5, g_bytes_new_static("1111", 5)), //
  //     "ToolTip", "test-tooltip",                                     //
  //     NULL);
}

Tray *tray_new(const char *dbus_name) {
  g_print("tray_new\n");
  GObject *object = g_object_new(tray_get_type(), NULL);
  Tray *tray = TRAY(object);
  tray->dbus_name = g_strdup(dbus_name);

  tray->owned_id = g_bus_own_name(G_BUS_TYPE_SESSION, tray->dbus_name,
                                  G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
                                  on_name_acquired, on_name_lost, tray, NULL);

  return tray;
}
