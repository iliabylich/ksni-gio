setup build:
    meson setup builddir --buildtype={{build}}

compile:
    meson compile -C builddir

run:
    ./builddir/example

clean:
    rm -rf builddir
