#ifndef KSNI_GIO_KSNI_H
#define KSNI_GIO_KSNI_H

#include <gio/gio.h>

G_DECLARE_FINAL_TYPE(Ksni, ksni, KSNI, KSNI, GObject)
#define KSNI(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, ksni_get_type(), Ksni)

Ksni *ksni_new(const char *id);
void ksni_register(Ksni *ksni, GDBusConnection *connection);
void ksni_unregister(Ksni *ksni, GDBusConnection *connection);

#endif // KSNI_GIO_KSNI_H
