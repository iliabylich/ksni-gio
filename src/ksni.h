#ifndef KSNI_GIO_KSNI_H
#define KSNI_GIO_KSNI_H

#include "api.h"
#include <gio/gio.h>

G_DECLARE_FINAL_TYPE(Ksni, ksni, KSNI, KSNI, GObject)
#define KSNI(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, ksni_get_type(), Ksni)

Ksni *ksni_new(void);

void ksni_update_title(Ksni *ksni, const char *title);
void ksni_update_icon_name(Ksni *ksni, const char *icon_name);
void ksni_update_icon_pixmap(Ksni *ksni, Pixmap *icon_pixmap);
void ksni_update_tooltip(Ksni *ksni, const char *tooltip);

gboolean ksni_start(Ksni *ksni, GDBusConnection *connection);

const char *ksni_get_dbus_name(Ksni *ksni);

#endif // KSNI_GIO_KSNI_H
