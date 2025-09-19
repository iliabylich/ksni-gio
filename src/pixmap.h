#ifndef PIXMAP_H
#define PIXMAP_H

#include "glib-object.h"
#include "glib.h"

G_DECLARE_FINAL_TYPE(Pixmap, pixmap, PIXMAP, PIXMAP, GObject)
#define PIXMAP(obj) G_TYPE_CHECK_INSTANCE_CAST(obj, pixmap_get_type(), Pixmap)

Pixmap *pixmap_new(guint w, guint h, GBytes *bytes);
GVariant *pixmap_to_gvariant(Pixmap *pixmap);

#endif // PIXMAP_H
