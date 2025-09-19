#ifndef TRAY_H
#define TRAY_H

#include "glib-object.h"

G_DECLARE_FINAL_TYPE(Tray, tray, TRAY, TRAY, GObject)
#define TRAY(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, tray_get_type(), Tray)

Tray *tray_new(const char *dbus_name);

void tray_trigger_test_error(Tray *tray);

#endif // TRAY_H
