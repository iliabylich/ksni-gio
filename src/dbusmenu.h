#ifndef DBUSMENU_H
#define DBUSMENU_H

#include "api.h"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

G_DECLARE_FINAL_TYPE(DBusMenu, dbusmenu, DBUSMENU, DBUSMENU, GObject)
#define DBUSMENU(obj)                                                          \
  G_TYPE_CHECK_INSTANCE_CAST(obj, dbusmenu_get_type(), DBusMenu)

DBusMenu *dbusmenu_new(void);
gboolean dbusmenu_start(DBusMenu *dbusmenu, GDBusConnection *connection);
void dbusmenu_update(DBusMenu *dbusmenu, dbusmenu_item_t *menu);

#endif // DBUSMENU_H
