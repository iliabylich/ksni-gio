#include "host.h"

struct _KsniHost {
  GObject parent_instance;

  GDBusConnection *connection;
  guint watch_id;
};

enum signal_types {
  SIGNAL_APPEARED = 0,
  LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE(KsniHost, ksni_host, G_TYPE_OBJECT)

static void ksni_host_class_init(KsniHostClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  signals[SIGNAL_APPEARED] = g_signal_new_class_handler(
      "appeared", G_OBJECT_CLASS_TYPE(object_class), G_SIGNAL_RUN_LAST, NULL,
      NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void ksni_host_init(KsniHost *ksni_host) { (void)ksni_host; }

KsniHost *ksni_host_new(void) {
  return g_object_new(ksni_host_get_type(), NULL);
}

static void on_host_appeared(GDBusConnection *connection, const gchar *name,
                             const gchar *name_owner, gpointer user_data) {
  (void)connection;
  KsniHost *ksni_host = KSNI_HOST(user_data);
  g_print("Host appeared: %s %s\n", name, name_owner);
  g_signal_emit(ksni_host, signals[SIGNAL_APPEARED], 0);
}

static void on_host_vanished(GDBusConnection *connection, const gchar *name,
                             gpointer user_data) {
  (void)connection;
  (void)name;
  (void)user_data;
}

void ksni_host_start_watching(KsniHost *ksni_host,
                              GDBusConnection *connection) {
  ksni_host->connection = connection;
  ksni_host->watch_id =
      g_bus_watch_name(G_BUS_TYPE_SESSION, "org.kde.StatusNotifierWatcher",
                       G_BUS_NAME_WATCHER_FLAGS_NONE, on_host_appeared,
                       on_host_vanished, ksni_host, NULL);
}

static void on_registration_completed(GObject *source_object, GAsyncResult *res,
                                      gpointer data) {
  (void)data;

  GError *error = NULL;
  GVariant *response = g_dbus_connection_call_finish(
      G_DBUS_CONNECTION(source_object), res, &error);

  if (error != NULL) {
    g_print("Registration failed: %s\n", error->message);
    g_error_free(error);
    return;
  }

  g_print("Registration completed successfully\n");
  g_variant_unref(response);
}

void ksni_host_register(KsniHost *ksni_host, const char *self_dbus_name) {
  g_dbus_connection_call(
      ksni_host->connection, "org.kde.StatusNotifierWatcher",
      "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher",
      "RegisterStatusNotifierItem", g_variant_new("(s)", self_dbus_name), NULL,
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, on_registration_completed, ksni_host);
}
