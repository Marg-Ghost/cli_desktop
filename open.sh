#!/bin/bash
read -r TYPE TARGET <<< $(./cli_desktop)

if [ "$TYPE" = "DIR" ]; then
    cd "$TARGET"
elif [ "$TYPE" = "FILE" ]; then
    nano "$TARGET"
fi