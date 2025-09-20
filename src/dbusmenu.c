#include "dbusmenu.h"
#include "com.canonical.dbusmenu.xml.null.xxd.h"
#include "dbusmenu_item-private.h"
#include "glib-object.h"
#include "glib.h"
#include "src/api.h"

struct _DBusMenu {
  GObject parent_instance;

  GDBusConnection *connection;
  GDBusNodeInfo *introspection;
  const char *unique_name;
  char *alias_name;
  guint object_registration_id;

  guint revision;
  dbusmenu_item_t *menu;
  dbusmenu_item_t *dummy;
  guint pending_layout_update_id;
};

enum signal_types {
  SIGNAL_ITEM_CLICK = 0,
  LAST_SIGNAL,
};
static guint signals[LAST_SIGNAL] = {0};

G_DEFINE_TYPE(DBusMenu, dbusmenu, G_TYPE_OBJECT)

static void dbusmenu_init(DBusMenu *dbusmenu) {
  GError *error = NULL;
  dbusmenu->introspection = g_dbus_node_info_new_for_xml(
      (const char *)com_canonical_dbusmenu_xml_null, &error);
  if (error != NULL) {
    g_printerr("failed to create intropspection object: %s\n", error->message);
    g_error_free(error);
  }

  dbusmenu->revision = 1;
  dbusmenu->menu = dbusmenu_item_new_root(0);
  dbusmenu->dummy = dbusmenu_item_new_root(0);
  dbusmenu->pending_layout_update_id = 0;
}

static void dbusmenu_dispose(GObject *object) {
  DBusMenu *dbusmenu = DBUSMENU(object);

  if (dbusmenu->object_registration_id > 0) {
    g_dbus_connection_unregister_object(dbusmenu->connection,
                                        dbusmenu->object_registration_id);
    dbusmenu->object_registration_id = 0;
  }

  if (dbusmenu->pending_layout_update_id > 0) {
    g_source_remove(dbusmenu->pending_layout_update_id);
    dbusmenu->pending_layout_update_id = 0;
  }

  g_clear_pointer(&dbusmenu->introspection, g_dbus_node_info_unref);
  g_clear_pointer(&dbusmenu->menu, dbusmenu_item_free);
  g_clear_pointer(&dbusmenu->dummy, dbusmenu_item_free);
  g_clear_pointer(&dbusmenu->alias_name, g_free);

  G_OBJECT_CLASS(dbusmenu_parent_class)->dispose(object);
}

static void dbusmenu_class_init(DBusMenuClass *klass) {
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  object_class->dispose = dbusmenu_dispose;

  signals[SIGNAL_ITEM_CLICK] = g_signal_new_class_handler(
      "item-click", G_OBJECT_CLASS_TYPE(object_class), G_SIGNAL_RUN_LAST, NULL,
      NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT);
}

static const GDBusInterfaceVTable interface_vtable;

static gboolean dbusmenu_register_object(DBusMenu *dbusmenu) {
  GError *error = NULL;
  dbusmenu->object_registration_id = g_dbus_connection_register_object(
      dbusmenu->connection, "/Menu", dbusmenu->introspection->interfaces[0],
      &interface_vtable, dbusmenu, NULL, &error);

  if (error != NULL) {
    g_printerr("failed to register DBus object for KSNI: %s\n", error->message);
    g_error_free(error);
    return FALSE;
  }
  g_print("Registration of /Menu has completed\n");
  return TRUE;
}

gboolean dbusmenu_start(DBusMenu *dbusmenu, GDBusConnection *connection) {
  dbusmenu->connection = connection;
  if (!dbusmenu_register_object(dbusmenu)) {
    return FALSE;
  }
  return TRUE;
}

static gboolean send_layout_updated_signal(void *user_data) {
  DBusMenu *dbusmenu = DBUSMENU(user_data);

  GError *error = NULL;
  guint revision = ++dbusmenu->revision;
  guint parent = dbusmenu->menu->id;
  g_dbus_connection_emit_signal(
      dbusmenu->connection, NULL, "/Menu", "com.canonical.dbusmenu",
      "LayoutUpdated", g_variant_new("(ui)", revision, parent), &error);
  if (error != NULL) {
    g_printerr("failed to send LayoutUpdated signal: %s\n", error->message);
    g_error_free(error);
  }

  dbusmenu->pending_layout_update_id = 0;
  return G_SOURCE_REMOVE;
}

void dbusmenu_update(DBusMenu *dbusmenu, dbusmenu_item_t *menu) {
  g_clear_pointer(&dbusmenu->menu, dbusmenu_item_free);
  dbusmenu->menu = menu;

  if (dbusmenu->connection && dbusmenu->pending_layout_update_id == 0) {
    dbusmenu->pending_layout_update_id =
        g_idle_add(send_layout_updated_signal, dbusmenu);
  }
}

DBusMenu *dbusmenu_new(void) { return g_object_new(dbusmenu_get_type(), NULL); }

// DBus interrface

static void dbusmenu_get_layout(DBusMenu *dbusmenu, GVariant *parameters,
                                GDBusMethodInvocation *invocation) {
  dbusmenu_item_t *menu = dbusmenu->menu;

  gint32 parent_id;
  gint32 recursion_depth;
  GVariant *property_names_variant;
  g_variant_get(parameters, "(ii@as)", &parent_id, &recursion_depth,
                &property_names_variant);
  g_variant_unref(property_names_variant);

  dbusmenu_item_t *subtree = dbusmenu_item_find_node(menu, parent_id);
  if (subtree == NULL) {
    subtree = dbusmenu->dummy;
  }
  GVariant *layout;

  if (recursion_depth == -1) {
    layout = dbusmenu_item_serialize(subtree);
  } else {
    dbusmenu_item_t *truncated =
        dbusmenu_item_clone_tree(subtree, recursion_depth);
    layout = dbusmenu_item_serialize(truncated);
    dbusmenu_item_free(truncated);
  }

  // gchar *layout_str =
  //     g_variant_print(layout, TRUE); // TRUE for type annotations
  // g_printerr("GetLayout variant: %s\n", layout_str);
  // g_printerr("GetLayout type: %s\n", g_variant_get_type_string(layout));

  GVariant *response =
      g_variant_new("(u@(ia{sv}av))", dbusmenu->revision, layout);
  g_dbus_method_invocation_return_value(invocation, response);
}

static void dbusmenu_get_group_properties(DBusMenu *dbusmenu,
                                          GVariant *parameters,
                                          GDBusMethodInvocation *invocation) {
  dbusmenu_item_t *menu = dbusmenu->menu;
  GVariant *ids_variant;
  GVariant *property_names_variant;
  g_variant_get(parameters, "(@ai@as)", &ids_variant, &property_names_variant);

  gsize n_ids;
  const gint32 *ids =
      g_variant_get_fixed_array(ids_variant, &n_ids, sizeof(gint32));

  gsize n_props;
  const gchar **props = g_variant_get_strv(property_names_variant, &n_props);

  GVariant *response =
      dbusmenu_item_serialize_selected(menu, n_props, props, n_ids, ids);

  // gchar *response_str =
  //     g_variant_print(response, TRUE); // TRUE for type annotations
  // g_printerr("GetGroupProperties variant: %s\n", response_str);
  // g_printerr("GetGroupProperties type: %s\n",
  //            g_variant_get_type_string(response));

  g_dbus_method_invocation_return_value(invocation,
                                        g_variant_new_tuple(&response, 1));

  g_free(props);
  g_variant_unref(ids_variant);
  g_variant_unref(property_names_variant);
}

static void dbusmenu_event_group(DBusMenu *dbusmenu, GVariant *parameters,
                                 GDBusMethodInvocation *invocation) {
  GVariant *events = g_variant_get_child_value(parameters, 0);
  gsize n_events = g_variant_n_children(events);

  for (gsize i = 0; i < n_events; i++) {
    GVariant *event = g_variant_get_child_value(events, i);

    gint32 id;
    const gchar *str_value;
    GVariant *variant_value;
    guint32 uint_value;

    g_variant_get(event, "(isvu)", &id, &str_value, &variant_value,
                  &uint_value);

    g_signal_emit(dbusmenu, signals[SIGNAL_ITEM_CLICK], 0, id);

    g_variant_unref(variant_value);
    g_variant_unref(event);
  }

  g_variant_unref(events);

  GVariant *errors = g_variant_new("ai", NULL);
  GVariant *response = g_variant_new_tuple(&errors, 1);

  g_dbus_method_invocation_return_value(invocation, response);
}

static void dbusmenu_event(DBusMenu *dbusmenu, GVariant *parameters,
                           GDBusMethodInvocation *invocation) {
  gint32 id;
  const gchar *event_id;
  GVariant *data;
  guint32 timestamp;
  g_variant_get(parameters, "(isvu)", &id, &event_id, &data, &timestamp);

  g_signal_emit(dbusmenu, signals[SIGNAL_ITEM_CLICK], 0, id);

  g_dbus_method_invocation_return_value(invocation, NULL);

  g_variant_unref(data);
}

static void
dbusmenu_on_method_call(GDBusConnection *connection, const gchar *sender,
                        const gchar *object_path, const gchar *interface_name,
                        const gchar *method_name, GVariant *parameters,
                        GDBusMethodInvocation *invocation, gpointer user_data) {
  (void)connection;
  (void)sender;
  (void)object_path;
  (void)interface_name;

  DBusMenu *dbusmenu = DBUSMENU(user_data);
  if (g_strcmp0(method_name, "GetLayout") == 0) {
    dbusmenu_get_layout(dbusmenu, parameters, invocation);
    return;
  } else if (g_strcmp0(method_name, "GetGroupProperties") == 0) {
    dbusmenu_get_group_properties(dbusmenu, parameters, invocation);
    return;
  } else if (g_strcmp0(method_name, "EventGroup") == 0) {
    dbusmenu_event_group(dbusmenu, parameters, invocation);
    return;
  } else if (g_strcmp0(method_name, "Event") == 0) {
    dbusmenu_event(dbusmenu, parameters, invocation);
    return;
  } else if (g_strcmp0(method_name, "AboutToShowGroup") == 0) {
    g_dbus_method_invocation_return_value(invocation, NULL);
    return;
  }

  g_print("DBusMenu unsupported method_call %s\n", method_name);
  g_dbus_method_invocation_return_value(invocation, NULL);
}

static GVariant *dbusmenu_on_get_property(GDBusConnection *connection,
                                          const gchar *sender,
                                          const gchar *object_path,
                                          const gchar *interface_name,
                                          const gchar *property_name,
                                          GError **error, gpointer user_data) {
  (void)connection;
  (void)sender;
  (void)object_path;
  (void)interface_name;
  (void)error;
  (void)user_data;

  // constant properties, required by spec
  if (g_strcmp0(property_name, "Version") == 0) {
    return g_variant_new_uint32(4);
  } else if (g_strcmp0(property_name, "Status") == 0) {
    return g_variant_new_string("normal");
  } else if (g_strcmp0(property_name, "TextDirection") == 0) {
    return g_variant_new_string("ltr");
  } else if (g_strcmp0(property_name, "IconThemePath") == 0) {
    return g_variant_new("as", NULL);
  }

  // this function only receives get-property requests for known fields
  // unknown fields are automatically handled by Gio.
  // Technically the next line is meant to be unreachable
  g_printerr("Unsupported DBusMenu property %s\n", property_name);
  return NULL;
}

static const GDBusInterfaceVTable interface_vtable = {
    .method_call = dbusmenu_on_method_call,
    .get_property = dbusmenu_on_get_property,
    .padding = {0},
};
