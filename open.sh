#!/bin/bash
while true; do
    OUTPUT=$(~/cli_desktop)

    ACTION=$(sed -n '1p' <<< "$OUTPUT")
    EDITOR_CHOICE=$(sed -n '2p' <<< "$OUTPUT")
    TARGET=$(sed -n '3p' <<< "$OUTPUT")

    if [ "$ACTION" = "QUIT" ] || [ -z "$ACTION" ]; then
        break
    fi

    if [ "$ACTION" = "D" ]; then
        rm -ri -- "$TARGET"
        continue
    fi
    if [ "$ACTION" = "A" ]; then
            touch "$TARGET"
            continue
    fi

    if [ -d "$TARGET" ]; then
        cd "$TARGET" || break
    elif [ -f "$TARGET" ]; then
        "$EDITOR_CHOICE" "$TARGET" < /dev/tty > /dev/tty
    fi
done