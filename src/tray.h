#ifndef TRAY_H
#define TRAY_H

#include "glib-object.h"
#include "pixmap.h"

G_DECLARE_FINAL_TYPE(Tray, tray, TRAY, TRAY, GObject)
#define TRAY(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, tray_get_type(), Tray)

Tray *tray_new(void);
void tray_update_title(Tray *tray, const char *title);
void tray_update_icon_name(Tray *tray, const char *icon_name);
void tray_update_icon_pixmap(Tray *tray, Pixmap *icon_pixmap);
void tray_update_tooltip(Tray *tray, const char *tooltip);

#endif // TRAY_H
