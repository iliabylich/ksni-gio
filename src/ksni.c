#include "ksni.h"
#include "org.kde.StatusNotifierItem.xml.null.xxd.h"
#include "pixmap.h"
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <string.h>

struct _Ksni {
  GObject parent_instance;

  GDBusNodeInfo *introspection;
  guint registration_id;

  char *id;
  char *title;
  char *icon_name;
  Pixmap *icon_pixmap;
  char *tooltip;
};

G_DEFINE_TYPE(Ksni, ksni, G_TYPE_OBJECT)

typedef enum {
  PROP_ID = 1,
  PROP_TITLE,
  PROP_ICON_NAME,
  PROP_ICON_PIXMAP,
  PROP_TOOLTIP,
  N_PROPERTIES,
} KsniProperty;
static GParamSpec *properties[N_PROPERTIES] = {0};

static void ksni_init(Ksni *self) {
  GError *error = NULL;
  self->introspection = g_dbus_node_info_new_for_xml(
      (const char *)org_kde_StatusNotifierItem_xml_null, &error);
  if (error != NULL) {
    g_printerr("failed to create intropspection object: %s\n", error->message);
    g_error_free(error);
  }
}

static void ksni_dispose(GObject *object) {
  g_print("KSNI dispose...\n");
  Ksni *self = KSNI(object);
  g_clear_pointer(&self->introspection, g_dbus_node_info_unref);
}

static void ksni_get_property(GObject *object, guint property_id, GValue *value,
                              GParamSpec *pspec) {
  Ksni *ksni = KSNI(object);

  switch ((KsniProperty)property_id) {
  case PROP_ID:
    g_value_set_string(value, ksni->id);
    break;

  case PROP_TITLE:
    g_value_set_string(value, ksni->title);
    break;

  case PROP_ICON_NAME:
    g_value_set_string(value, ksni->icon_name);
    break;

  case PROP_ICON_PIXMAP:
    g_value_set_object(value, ksni->icon_pixmap);
    break;

  case PROP_TOOLTIP:
    g_value_set_string(value, ksni->tooltip);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void ksni_set_property(GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec) {
  Ksni *ksni = KSNI(object);

  switch ((KsniProperty)property_id) {
  case PROP_ID:
    g_clear_pointer(&ksni->id, g_free);
    ksni->id = g_value_dup_string(value);
    break;

  case PROP_TITLE:
    g_clear_pointer(&ksni->title, g_free);
    ksni->title = g_value_dup_string(value);
    break;

  case PROP_ICON_NAME:
    g_clear_pointer(&ksni->icon_name, g_free);
    ksni->icon_name = g_value_dup_string(value);
    break;

  case PROP_ICON_PIXMAP:
    g_clear_pointer(&ksni->icon_pixmap, g_object_unref);
    ksni->icon_pixmap = g_value_get_object(value);
    break;

  case PROP_TOOLTIP:
    g_clear_pointer(&ksni->tooltip, g_free);
    ksni->tooltip = g_value_dup_string(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    break;
  }
}

static void ksni_class_init(KsniClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = ksni_dispose;

  object_class->get_property = ksni_get_property;
  object_class->set_property = ksni_set_property;

  properties[PROP_ID] =
      g_param_spec_string("Id", NULL, NULL, NULL, G_PARAM_READWRITE);
  properties[PROP_TITLE] =
      g_param_spec_string("Title", NULL, NULL, NULL, G_PARAM_READWRITE);
  properties[PROP_ICON_NAME] =
      g_param_spec_string("IconName", NULL, NULL, NULL, G_PARAM_READWRITE);
  properties[PROP_ICON_PIXMAP] = g_param_spec_object(
      "IconPixmap", NULL, NULL, G_TYPE_OBJECT, G_PARAM_READWRITE);
  properties[PROP_TOOLTIP] =
      g_param_spec_string("ToolTip", NULL, NULL, NULL, G_PARAM_READWRITE);

  g_object_class_install_properties(object_class, N_PROPERTIES, properties);
}

Ksni *ksni_new(const char *id) {
  g_print("ksni_new\n");
  GObject *object = g_object_new(ksni_get_type(), "Id", id, NULL);
  Ksni *ksni = KSNI(object);

  return ksni;
}

static const GDBusInterfaceVTable interface_vtable;

void ksni_register(Ksni *ksni, GDBusConnection *connection) {
  GError *error = NULL;

  GDBusInterfaceInfo *interface_info = NULL;
  if (ksni->introspection != NULL) {
    interface_info = ksni->introspection->interfaces[0];
  }

  ksni->registration_id = g_dbus_connection_register_object(
      connection, "/StatusNotifierItem", interface_info, &interface_vtable,
      ksni, NULL, &error);
  if (error != NULL) {
    g_printerr("failed to register DBus object for KSNI: %s\n", error->message);
    g_error_free(error);
  }
}

void ksni_unregister(Ksni *ksni, GDBusConnection *connection) {
  g_dbus_connection_unregister_object(connection, ksni->registration_id);
}

static void ksni_on_method_call(GDBusConnection *connection,
                                const gchar *sender, const gchar *object_path,
                                const gchar *interface_name,
                                const gchar *method_name, GVariant *parameters,
                                GDBusMethodInvocation *invocation,
                                gpointer user_data) {
  (void)connection;
  (void)sender;
  (void)object_path;
  (void)interface_name;
  (void)method_name;
  (void)parameters;
  (void)user_data;
  g_dbus_method_invocation_return_value(invocation, NULL);
}

static GVariant *ksni_on_get_property(GDBusConnection *connection,
                                      const gchar *sender,
                                      const gchar *object_path,
                                      const gchar *interface_name,
                                      const gchar *property_name,
                                      GError **error, gpointer user_data) {
  (void)connection;
  (void)sender;
  (void)object_path;
  (void)interface_name;
  (void)property_name;
  (void)error;

  // constant properties, required by spec
  if (g_strcmp0(property_name, "Category") == 0) {
    return g_variant_new_string("ApplicationStatus");
  } else if (g_strcmp0(property_name, "Status") == 0) {
    return g_variant_new_string("Active");
  } else if (g_strcmp0(property_name, "WindowId") == 0) {
    return g_variant_new_int32(0);
  } else if (g_strcmp0(property_name, "OverlayIconName") == 0) {
    return g_variant_new_string("");
  } else if (g_strcmp0(property_name, "OverlayIconPixmap") == 0) {
    return pixmap_to_gvariant(NULL);
  } else if (g_strcmp0(property_name, "AttentionIconName") == 0) {
    return g_variant_new_string("");
  } else if (g_strcmp0(property_name, "AttentionIconPixmap") == 0) {
    return pixmap_to_gvariant(NULL);
  } else if (g_strcmp0(property_name, "AttentionMovieName") == 0) {
    return g_variant_new_string("");
  } else if (g_strcmp0(property_name, "ItemIsMenu") == 0) {
    return g_variant_new_boolean(FALSE);
  } else if (g_strcmp0(property_name, "Menu") == 0) {
    return g_variant_new_object_path("/MenuBar");
  } else if (g_strcmp0(property_name, "IconThemePath") == 0) {
    return g_variant_new_string("");
  }

  Ksni *ksni = KSNI(user_data);
  GValue gvalue = G_VALUE_INIT;
  g_object_get_property(G_OBJECT(ksni), property_name, &gvalue);

  if (g_strcmp0(property_name, "Id") == 0) {
    const char *id = ksni->id;
    if (id == NULL) {
      id = "";
    }
    return g_variant_new_string(id);
  } else if (g_strcmp0(property_name, "Title") == 0) {
    const char *title = ksni->title;
    if (title == NULL) {
      title = "";
    }
    return g_variant_new_string(title);
  } else if (g_strcmp0(property_name, "IconName") == 0) {
    const char *icon_name = ksni->icon_name;
    if (icon_name == NULL) {
      icon_name = "";
    }
    return g_variant_new_string(icon_name);
  } else if (g_strcmp0(property_name, "IconPixmap") == 0) {
    Pixmap *pixmap = g_value_get_object(&gvalue);
    return pixmap_to_gvariant(pixmap);
  } else if (g_strcmp0(property_name, "ToolTip") == 0) {
    const char *tooltip = ksni->tooltip;
    if (tooltip == NULL) {
      tooltip = "";
    }
    return g_variant_new("(sa(iiay)ss)", tooltip, NULL, "", "");
  }

  // this function only receives get-property requests for known fields
  // unknown fields are automatically handled by Gio.
  // Technically the next line is meant to be unreachable
  g_printerr("Unsupported KSNI property %s\n", property_name);
  return NULL;
}

static const GDBusInterfaceVTable interface_vtable = {
    .method_call = ksni_on_method_call,
    .get_property = ksni_on_get_property,
    .padding = {0},
};
