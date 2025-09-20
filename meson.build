project(
    'ksni-gio',
    'c',
    version: '1.0.0',
    default_options: ['warning_level=3', 'c_std=c99'],
)

buildtype = get_option('buildtype')
if buildtype == 'debug'
    add_project_arguments('-fsanitize=address', language: 'c')
    add_project_link_arguments('-fsanitize=address', language: 'c')
endif

xxd = generator(
    find_program('xxd'),
    output: '@PLAINNAME@.xxd.h',
    arguments: ['-n', '@PLAINNAME@', '-i', '@INPUT@', '@OUTPUT@'],
)
append_null = generator(
    find_program('sh'),
    arguments: ['-c', 'cat "@INPUT@" && dd if=/dev/zero bs=1 count=1 2>/dev/null'],
    output: '@PLAINNAME@.null',
    capture: true,
)
generated = [
    xxd.process(append_null.process('xml/org.kde.StatusNotifierItem.xml')),
    xxd.process(append_null.process('xml/com.canonical.dbusmenu.xml')),
]

ksni_gio = shared_library(
    'ksni-gio',
    files(
        'src/dbusmenu.c',
        'src/dbusmenu_item.c',
        'src/ksni.c',
        'src/ksni_host.c',
        'src/pixmap.c',
        'src/tray.c',
    )
    + generated,
    dependencies: [
        dependency('gio-2.0'),
    ],
)

icons = [
    xxd.process('icons/clouds.argb'),
    xxd.process('icons/sun.argb'),
]

executable(
    'example',
    files('examples/main.c') + icons,
    link_with: [ksni_gio],
    include_directories: [
        include_directories('src'),
    ],
    dependencies: [
        dependency('glib-2.0'),
        dependency('gobject-2.0'),
    ],
)
