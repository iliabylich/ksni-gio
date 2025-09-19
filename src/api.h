#ifndef TRAY_API_H
#define TRAY_API_H

#include <glib-object.h>

G_DECLARE_FINAL_TYPE(Tray, tray, TRAY, TRAY, GObject)
#define TRAY(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, tray_get_type(), Tray)

G_DECLARE_FINAL_TYPE(Pixmap, pixmap, PIXMAP, PIXMAP, GObject)
#define PIXMAP(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, pixmap_get_type(), Pixmap)

Tray *tray_new(void);

Pixmap *pixmap_new(guint w, guint h, GBytes *bytes);
GVariant *pixmap_to_gvariant(Pixmap *pixmap);

typedef struct dbusmenu_item_t dbusmenu_item_t;
dbusmenu_item_t *dbusmenu_item_new_root(guint id);
dbusmenu_item_t *dbusmenu_item_new_standard(guint id, const char *label,
                                            gboolean enabled, gboolean visible);
dbusmenu_item_t *dbusmenu_item_new_separator(guint id, gboolean visible);
dbusmenu_item_t *dbusmenu_item_new_checkbox(guint id, const char *label,
                                            gboolean enabled, gboolean visible,
                                            gboolean checked);
dbusmenu_item_t *dbusmenu_item_new_radio(guint id, const char *label,
                                         gboolean enabled, gboolean visible,
                                         gboolean checked);
dbusmenu_item_t *dbusmenu_item_new_submenu(guint id, const char *label,
                                           gboolean visible);
void dbusmenu_item_submenu_push_child(dbusmenu_item_t *parent,
                                      dbusmenu_item_t *child);

void tray_update_title(Tray *tray, const char *title);
void tray_update_icon_name(Tray *tray, const char *icon_name);
void tray_update_icon_pixmap(Tray *tray, Pixmap *icon_pixmap);
void tray_update_tooltip(Tray *tray, const char *tooltip);
void tray_update_menu(Tray *tray, dbusmenu_item_t *menu);

#endif // TRAY_API_H
