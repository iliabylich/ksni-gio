# KSNI implementation using Gio only

Implements:

1. `org.kde.StatusNotifierItem` (KSNI)
2. `com.canonical.dbusmenu`

Supports:

1. from KSNI:
    1. `Title`
    2. `ToolTip`
    3. `IconName`
    4. `IconPixmap`
    5. handling `"click"` event
2. from dbusmenu:
    1. full layout updates via `LayoutUpdated` (i.e. no incremental updates via `ItemsPropertiesUpdated`)
    2. Different layout items: "standard", "radio", "checbox", "submenu"
    3. handling `"item-click"` event on individual menu items

### API

Can be found in [`src/ksni-gio.h`](/src/ksni-gio.h).

On top of that `Tray` object has 2 signals:

1. `"click"` - when tray icon is clicked
2. `"item-click"` - when menu item is clicked, has `item_id` as an argument

### Building

```sh
meson setup builddir --buildtype=debug # or "release"
meson compile -C builddir
```

### Demo example

After compiling the code you'll have `./builddir/example` file. Make sure you have something in your system that is able to show a tray icon (e.g. Waybar).

Environment variables:

1. `USE_ICON_PIXMAP=1` - makes example use icons from `icons/` directory. If not specified, `IconName` is used instead (`edit-copy` or `edit-delete` from freedesktop specification)
2. `DO_CHANGES_AUTOMATICALLY=1` - changes `Title` + `IconName`/`IconPixmap` (depending on the previous choice) + `Menu` every 100ms.
3. `TEST_EXIT=1` - makes the process do a graceful shutdown with normal cleanup after 2 seconds.

### Memory leaks

The code has been tested for memory leaks using Address Sanitizer. Building in debug mode automatically enables Address Sanitizer, and by combining it with `TEST_EXIT=1` (for proper cleanup), `DO_CHANGES_AUTOMATICALLY=1` for stress testing, and conditional usage of `USE_ICON_PIXMAP=1` all happy paths can be covered.

There's a chance that some error paths produce memory leaks but it's difficult to test all edge cases.

### Credits

Icons for demo are taken from [FlatIcon](https://flaticon.com)
