#include "pixmap.h"
#include "glib.h"
#include <glib-object.h>

struct _Pixmap {
  GObject parent_instance;

  guint w;
  guint h;
  GBytes *bytes;
};

G_DEFINE_TYPE(Pixmap, pixmap, G_TYPE_OBJECT)

typedef enum {
  PROP_W = 1,
  PROP_H,
  PROP_BYTES,
  N_PROPERTIES,
} PropertyId;
static GParamSpec *properties[N_PROPERTIES] = {0};

static void pixmap_init(Pixmap *pixmap) { (void)pixmap; }

static void pixmap_dispose(GObject *object) {
  g_print("PIXMAP dispose...\n");
  Pixmap *pixmap = PIXMAP(object);
  g_clear_pointer(&pixmap->bytes, g_bytes_unref);
}

static void pixmap_get_property(GObject *object, guint property_id,
                                GValue *value, GParamSpec *pspec) {
  Pixmap *pixmap = PIXMAP(object);

  switch ((PropertyId)property_id) {
  case PROP_W:
    g_value_set_uint(value, pixmap->w);
    break;

  case PROP_H:
    g_value_set_uint(value, pixmap->h);
    break;

  case PROP_BYTES:
    g_value_set_boxed(value, pixmap->bytes);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pixmap_set_property(GObject *object, guint property_id,
                                const GValue *value, GParamSpec *pspec) {
  Pixmap *pixmap = PIXMAP(object);

  switch ((PropertyId)property_id) {
  case PROP_W:
    pixmap->w = g_value_get_uint(value);
    break;

  case PROP_H:
    pixmap->h = g_value_get_uint(value);
    break;

  case PROP_BYTES:
    if (pixmap->bytes) {
      g_bytes_unref(pixmap->bytes);
    }
    pixmap->bytes = g_value_get_boxed(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void pixmap_class_init(PixmapClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = pixmap_dispose;

  object_class->get_property = pixmap_get_property;
  object_class->set_property = pixmap_set_property;

  properties[PROP_W] =
      g_param_spec_uint("w", NULL, NULL, 0, 100000, 0, G_PARAM_READWRITE);
  properties[PROP_H] =
      g_param_spec_uint("h", NULL, NULL, 0, 100000, 0, G_PARAM_READWRITE);
  properties[PROP_BYTES] =
      g_param_spec_boxed("bytes", NULL, NULL, G_TYPE_BYTES, G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, properties);
}

Pixmap *pixmap_new(guint w, guint h, GBytes *bytes) {
  g_print("pixmap_new\n");
  return g_object_new(   //
      pixmap_get_type(), //
      "w", w,            //
      "h", h,            //
      "bytes", bytes, NULL);
}

GVariant *pixmap_to_gvariant(Pixmap *pixmap) {
  if (pixmap == NULL) {
    return g_variant_new_array(G_VARIANT_TYPE("(iiay)"), NULL, 0);
  }

  gsize size;
  gconstpointer data = g_bytes_get_data(pixmap->bytes, &size);

  GVariant *item = g_variant_new(
      "(ii@ay)", pixmap->w, pixmap->h,
      g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, data, size, 1));

  GVariant *array = g_variant_new_array(G_VARIANT_TYPE("(iiay)"), &item, 1);

  return array;
}
