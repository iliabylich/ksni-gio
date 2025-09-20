setup build:
    meson setup builddir --buildtype={{build}}

compile:
    meson compile -C builddir

run:
    ./builddir/example

clean:
    rm -rf builddir

generate-icons:
    @just _generate-icon sun
    @just _generate-icon clouds

_generate-icon name:
    cd icons && magick {{ name }}.png -color-matrix "1 0 0 0, 1 0 0 0, 0 1 0 0, 0 0 1 0" -depth 8 -resize 32x32! rgba:{{ name }}.argb
