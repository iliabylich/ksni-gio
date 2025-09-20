#ifndef DBUSMENU_ITEM_PRIVATE_H
#define DBUSMENU_ITEM_PRIVATE_H

#include "ksni-gio.h"

typedef enum {
  DBUSMENU_ITEM_TYPE_STANDARD,
  DBUSMENU_ITEM_TYPE_SEPARATOR,
} dbusmenu_item_type_t;

typedef enum {
  DBUSMENU_ITEM_TOGGLE_TYPE_NONE,
  DBUSMENU_ITEM_TOGGLE_TYPE_CHECKMARK,
  DBUSMENU_ITEM_TOGGLE_TYPE_RADIO,
} dbusmenu_item_toggle_type_t;

struct dbusmenu_item_t {
  guint id;

  // "standard" (default) / "separator"
  dbusmenu_item_type_t type;

  char *label;

  gboolean enabled;

  gboolean visible;

  // "" (default) / "checkmark" / "radio"
  dbusmenu_item_toggle_type_t toggle_type;

  // 0 (off) / 1 (on)
  gboolean toggle_state;

  // "" (default) / "submenu" (it item has children)
  gboolean children_display;

  struct dbusmenu_item_t **children;
  guint children_count;
};

GVariant *dbusmenu_item_serialize(dbusmenu_item_t *item);
GVariant *dbusmenu_item_serialize_selected(dbusmenu_item_t *item, gsize n_props,
                                           const gchar **props, gsize n_ids,
                                           const gint32 *ids);
void dbusmenu_item_free(dbusmenu_item_t *item);
dbusmenu_item_t *dbusmenu_item_find_node(dbusmenu_item_t *item, guint id);
dbusmenu_item_t *dbusmenu_item_clone_tree(dbusmenu_item_t *item, guint depth);

#endif // DBUSMENU_ITEM_PRIVATE_H
