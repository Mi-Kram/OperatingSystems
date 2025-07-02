#!/bin/bash

clear

DIR="/tmp/kramarenko-dz13"
BASE="a"

rm -rf "$DIR"
mkdir "$DIR" || { echo "failed to create temp directory $DIR"; exit 1; }

# create base file
touch "$DIR/$BASE" || { echo "failed to create base file $DIR/$BASE"; exit 1; }

target="$BASE"
depth=0

while true; do
    linkname="link_${depth}"
    linkpath="$DIR/$linkname"

    # create link.
    ln -s "$target" "$linkpath" || { echo "failed to create a link $linkpath"; break; }

    # try to open link
    if ! exec {fd}<"$linkpath" 2>/dev/null; then
        echo "Max depth: $depth"
        rm -f "$linkpath"
        break
    fi
    exec {fd}<&- # close descriptor.

    target="$linkname"
    depth=$((depth + 1))
done

rm -rf "$DIR"
