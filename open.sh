#!/bin/bash
while true; do
    # Ausgabe holen und Trimmen
    TARGET=$(~/cli_desktop)

    # Abbrechen bei Leerzeile / q
    if [ -z "$TARGET" ]; then
        break
    fi

    if [ -d "$TARGET" ]; then
        cd "$TARGET" || break
    elif [ -f "$TARGET" ]; then
        # /dev/tty zwingt nano, das echte Terminal als Input/Output zu nutzen
        nano "$TARGET" < /dev/tty > /dev/tty
    else
        break
    fi
done