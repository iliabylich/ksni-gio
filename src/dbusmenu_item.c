#include "api.h"
#include "dbusmenu_item-private.h"
#include "glib.h"
#include <string.h>

dbusmenu_item_t *dbusmenu_item_new_root(guint id) {
  return dbusmenu_item_new_submenu(id, NULL, TRUE);
}

dbusmenu_item_t *dbusmenu_item_new_standard(guint id, const char *label,
                                            gboolean enabled,
                                            gboolean visible) {
  dbusmenu_item_t *item = malloc(sizeof(dbusmenu_item_t));
  item->id = id;
  item->type = DBUSMENU_ITEM_TYPE_STANDARD;
  item->label = g_strdup(label);
  item->enabled = enabled;
  item->visible = visible;
  item->toggle_type = DBUSMENU_ITEM_TOGGLE_TYPE_NONE;
  item->toggle_state = FALSE;
  item->children_display = FALSE;
  item->children = NULL;
  item->children_count = 0;
  return item;
}

dbusmenu_item_t *dbusmenu_item_new_separator(guint id, gboolean visible) {
  dbusmenu_item_t *item = malloc(sizeof(dbusmenu_item_t));
  item->id = id;
  item->type = DBUSMENU_ITEM_TYPE_SEPARATOR;
  item->label = NULL;
  item->enabled = TRUE;
  item->visible = visible;
  item->toggle_type = DBUSMENU_ITEM_TOGGLE_TYPE_NONE;
  item->toggle_state = FALSE;
  item->children_display = FALSE;
  item->children = NULL;
  item->children_count = 0;
  return item;
}

dbusmenu_item_t *dbusmenu_item_new_checkbox(guint id, const char *label,
                                            gboolean enabled, gboolean visible,
                                            gboolean checked) {
  dbusmenu_item_t *item = malloc(sizeof(dbusmenu_item_t));
  item->id = id;
  item->type = DBUSMENU_ITEM_TYPE_STANDARD;
  item->label = g_strdup(label);
  item->enabled = enabled;
  item->visible = visible;
  item->toggle_type = DBUSMENU_ITEM_TOGGLE_TYPE_CHECKMARK;
  item->toggle_state = checked;
  item->children_display = FALSE;
  item->children = NULL;
  item->children_count = 0;
  return item;
}

dbusmenu_item_t *dbusmenu_item_new_radio(guint id, const char *label,
                                         gboolean enabled, gboolean visible,
                                         gboolean checked) {
  dbusmenu_item_t *item = malloc(sizeof(dbusmenu_item_t));
  item->id = id;
  item->type = DBUSMENU_ITEM_TYPE_STANDARD;
  item->label = g_strdup(label);
  item->enabled = enabled;
  item->visible = visible;
  item->toggle_type = DBUSMENU_ITEM_TOGGLE_TYPE_RADIO;
  item->toggle_state = checked;
  item->children_display = FALSE;
  item->children = NULL;
  item->children_count = 0;
  return item;
}

dbusmenu_item_t *dbusmenu_item_new_submenu(guint id, const char *label,
                                           gboolean visible) {
  dbusmenu_item_t *item = malloc(sizeof(dbusmenu_item_t));
  item->id = id;
  item->type = DBUSMENU_ITEM_TYPE_STANDARD;
  item->label = g_strdup(label);
  item->enabled = TRUE;
  item->visible = visible;
  item->toggle_type = DBUSMENU_ITEM_TOGGLE_TYPE_NONE;
  item->toggle_state = FALSE;
  item->children_display = TRUE;
  item->children = NULL;
  item->children_count = 0;
  return item;
}

void dbusmenu_item_submenu_push_child(dbusmenu_item_t *parent,
                                      dbusmenu_item_t *child) {
  guint count = parent->children_count;
  dbusmenu_item_t **new_children = malloc(sizeof(void *) * (count + 1));
  if (parent->children != NULL) {
    memcpy(new_children, parent->children, count * sizeof(void *));
    free(parent->children);
  }
  parent->children = new_children;
  parent->children[parent->children_count] = child;
  parent->children_count++;
}

void dbusmenu_item_free(dbusmenu_item_t *item) {
  if (item->children_count > 0) {
    for (guint i = 0; i < item->children_count; i++) {
      dbusmenu_item_t *child = item->children[i];
      dbusmenu_item_free(child);
    }
    free(item->children);
  }
  if (item->label != NULL) {
    g_free(item->label);
  }
  free(item);
}

dbusmenu_item_t *dbusmenu_item_find_node(dbusmenu_item_t *item, guint id) {
  if (item->id == id) {
    return item;
  }
  for (guint i = 0; i < item->children_count; i++) {
    dbusmenu_item_t *scope = item->children[i];
    dbusmenu_item_t *found = dbusmenu_item_find_node(scope, id);
    if (found != NULL) {
      return found;
    }
  }

  return NULL;
}

dbusmenu_item_t *dbusmenu_item_clone_tree(dbusmenu_item_t *item, guint depth) {
  dbusmenu_item_t *clone = malloc(sizeof(dbusmenu_item_t));
  memcpy(clone, item, sizeof(dbusmenu_item_t));

  if (item->label != NULL) {
    clone->label = g_strdup(item->label);
  }

  clone->children = malloc(item->children_count * sizeof(void *));
  for (guint i = 0; i < item->children_count; i++) {
    dbusmenu_item_t *child = item->children[i];
    clone->children[i] = dbusmenu_item_clone_tree(child, depth - 1);
  }

  return clone;
}

static const char *item_type_to_string(dbusmenu_item_type_t type) {
  switch (type) {
  case DBUSMENU_ITEM_TYPE_STANDARD:
    return "standard";
  case DBUSMENU_ITEM_TYPE_SEPARATOR:
    return "separator";
  }
}

static const char *
toggle_type_to_string(dbusmenu_item_toggle_type_t toggle_type) {
  switch (toggle_type) {
  case DBUSMENU_ITEM_TOGGLE_TYPE_CHECKMARK:
    return "checkmark";
  case DBUSMENU_ITEM_TOGGLE_TYPE_RADIO:
    return "radio";
  case DBUSMENU_ITEM_TOGGLE_TYPE_NONE:
    return "";
  }
}

GVariant *dbusmenu_item_serialize(dbusmenu_item_t *item) {
  g_printerr("Serializing item %p\n", item);

  GVariantBuilder props_builder;
  g_variant_builder_init(&props_builder, G_VARIANT_TYPE("a{sv}"));

  g_variant_builder_add(&props_builder, "{sv}", "type",
                        g_variant_new_string(item_type_to_string(item->type)));
  g_variant_builder_add(&props_builder, "{sv}", "label",
                        g_variant_new_string(item->label ? item->label : ""));
  g_variant_builder_add(&props_builder, "{sv}", "enabled",
                        g_variant_new_boolean(item->enabled));
  g_variant_builder_add(&props_builder, "{sv}", "visible",
                        g_variant_new_boolean(item->visible));
  g_variant_builder_add(
      &props_builder, "{sv}", "toggle-type",
      g_variant_new_string(toggle_type_to_string(item->toggle_type)));
  g_variant_builder_add(&props_builder, "{sv}", "toggle-state",
                        g_variant_new_int32(item->toggle_state ? 1 : 0));
  g_variant_builder_add(
      &props_builder, "{sv}", "children-display",
      g_variant_new_string(item->children_count > 0 ? "submenu" : ""));

  GVariantBuilder children_builder;
  g_variant_builder_init(&children_builder, G_VARIANT_TYPE("av"));

  for (guint i = 0; i < item->children_count; i++) {
    GVariant *child_variant = dbusmenu_item_serialize(item->children[i]);
    if (child_variant) {
      g_variant_builder_add(&children_builder, "v", child_variant);
    }
  }

  return g_variant_new("(ia{sv}av)", (gint32)item->id, &props_builder,
                       &children_builder);
}

#define PROP_TYPE 1 << 0
#define PROP_LABEL 1 << 1
#define PROP_ENABLED 1 << 2
#define PROP_VISIBLE 1 << 3
#define PROP_TOGGLE_TYPE 1 << 4
#define PROP_TOGGLE_STATE 1 << 5
#define PROP_CHILDREN_DISPLAY 1 << 6
#define PROP_ALL                                                               \
  PROP_TYPE | PROP_LABEL | PROP_ENABLED | PROP_VISIBLE | PROP_TOGGLE_TYPE |    \
      PROP_TOGGLE_STATE | PROP_CHILDREN_DISPLAY

static int props_to_mask(gsize n_props, const gchar **props) {
  if (n_props == 0) {
    return PROP_ALL;
  }

  int mask = 0;
  for (gsize i = 0; i < n_props; i++) {
    const char *prop = props[i];
    if (g_strcmp0(prop, "type") == 0) {
      mask |= PROP_TYPE;
    } else if (g_strcmp0(prop, "label") == 0) {
      mask |= PROP_LABEL;
    } else if (g_strcmp0(prop, "enabled") == 0) {
      mask |= PROP_ENABLED;
    } else if (g_strcmp0(prop, "visible") == 0) {
      mask |= PROP_VISIBLE;
    } else if (g_strcmp0(prop, "toggle-type") == 0) {
      mask |= PROP_TOGGLE_TYPE;
    } else if (g_strcmp0(prop, "toggle-state") == 0) {
      mask |= PROP_TOGGLE_STATE;
    } else if (g_strcmp0(prop, "children-display") == 0) {
      mask |= PROP_CHILDREN_DISPLAY;
    } else {
      g_printerr("Unknown property %s\n", prop);
    }
  }
  return mask;
}

typedef void (*foreach_callback_t)(dbusmenu_item_t *item, void *data);

static void dbusmenu_item_foreach(dbusmenu_item_t *item,
                                  foreach_callback_t callback, void *data) {
  callback(item, data);
  for (guint i = 0; i < item->children_count; i++) {
    dbusmenu_item_t *child = item->children[i];
    dbusmenu_item_foreach(child, callback, data);
  }
}

static gboolean is_in(gint32 id, gsize n_ids, const gint32 *ids) {
  for (gsize i = 0; i < n_ids; i++) {
    if (ids[i] == id) {
      return TRUE;
    }
  }
  return FALSE;
}

typedef struct {
  int mask;
  gsize n_ids;
  const gint32 *ids;
  GVariantBuilder *builder;
} dbusmenu_item_serialize_selected0_callback_data_t;

void dbusmenu_item_serialize_item_if_selected(dbusmenu_item_t *item,
                                              void *data) {
  dbusmenu_item_serialize_selected0_callback_data_t *ctx = data;

  if (!is_in(item->id, ctx->n_ids, ctx->ids)) {
    return;
  }

  GVariantBuilder props_builder;
  g_variant_builder_init(&props_builder, G_VARIANT_TYPE("a{sv}"));

  if (ctx->mask & PROP_TYPE) {
    g_variant_builder_add(
        &props_builder, "{sv}", "type",
        g_variant_new_string(item_type_to_string(item->type)));
  }
  if (ctx->mask & PROP_LABEL) {
    g_variant_builder_add(&props_builder, "{sv}", "label",
                          g_variant_new_string(item->label ? item->label : ""));
  }
  if (ctx->mask & PROP_ENABLED) {
    g_variant_builder_add(&props_builder, "{sv}", "enabled",
                          g_variant_new_boolean(item->enabled));
  }
  if (ctx->mask & PROP_VISIBLE) {
    g_variant_builder_add(&props_builder, "{sv}", "visible",
                          g_variant_new_boolean(item->visible));
  }
  if (ctx->mask & PROP_TOGGLE_TYPE) {
    g_variant_builder_add(
        &props_builder, "{sv}", "toggle-type",
        g_variant_new_string(toggle_type_to_string(item->toggle_type)));
  }
  if (ctx->mask & PROP_TOGGLE_STATE) {
    g_variant_builder_add(&props_builder, "{sv}", "toggle-state",
                          g_variant_new_int32(item->toggle_state ? 1 : 0));
  }
  if (ctx->mask & PROP_CHILDREN_DISPLAY) {
    g_variant_builder_add(
        &props_builder, "{sv}", "children-display",
        g_variant_new_string(item->children_count > 0 ? "submenu" : ""));
  }

  GVariant *props = g_variant_builder_end(&props_builder);
  GVariant *id = g_variant_new_int32(item->id);

  GVariant *children[] = {id, props};
  GVariant *tuple = g_variant_new_tuple(children, 2);
  g_variant_builder_add_value(ctx->builder, tuple);
}

void dbusmenu_item_serialize_selected0(
    dbusmenu_item_t *item,
    dbusmenu_item_serialize_selected0_callback_data_t *data) {
  dbusmenu_item_foreach(item, dbusmenu_item_serialize_item_if_selected, data);
}

GVariant *dbusmenu_item_serialize_selected(dbusmenu_item_t *item, gsize n_props,
                                           const gchar **props, gsize n_ids,
                                           const gint32 *ids) {
  int mask = props_to_mask(n_props, props);

  GVariantBuilder builder;
  g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ia{sv})"));

  dbusmenu_item_serialize_selected0_callback_data_t data = {
      .mask = mask,
      .n_ids = n_ids,
      .ids = ids,
      .builder = &builder,
  };

  dbusmenu_item_serialize_selected0(item, &data);

  return g_variant_builder_end(data.builder);
}
