#ifndef HOST_H
#define HOST_H

#include <gio/gio.h>

G_DECLARE_FINAL_TYPE(KsniHost, ksni_host, KSNI_HOST, KSNI_HOST, GObject)
#define KSNI_HOST(obj)                                                         \
  G_TYPE_CHECK_INSTANCE_CAST(obj, ksni_host_get_type(), KsniHost)

KsniHost *ksni_host_new(void);
void ksni_host_start_watching(KsniHost *ksni_host, GDBusConnection *connection);
void ksni_host_register(KsniHost *ksni_host, const char *self_dbus_name);

#endif // HOST_H
