Bash
#!/bin/bash
while true; do
    TARGET=$(~/cli_desktop) #execute
    # Beenden, wenn 'q' ge
    if [ -z "$TARGET" ]; then
        break
    fi


    if [ -d "$TARGET" ]; then
        cd "$TARGET"
    elif [ -f "$TARGET" ]; then
        nano "$TARGET"
    fi
done