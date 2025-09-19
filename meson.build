project(
    'tray',
    'c',
    version: '1.0.0',
    default_options: ['warning_level=3', 'c_std=c99'],
)

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

tray = shared_library(
    'tray',
    files(
        'src/host.c',
        'src/ksni.c',
        'src/pixmap.c',
        'src/tray.c',
    )
    + xxd.process(append_null.process('xml/org.kde.StatusNotifierItem.xml')),
    dependencies: [
        dependency('gio-2.0'),
    ],
)

executable(
    'example',
    files('examples/main.c'),
    link_with: [tray],
    include_directories: [
        include_directories('src'),
    ],
    dependencies: [
        dependency('glib-2.0'),
        dependency('gobject-2.0'),
    ],
)
