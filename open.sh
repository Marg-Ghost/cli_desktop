#!/bin/bash
TARGET=$(./cli_desktop)

if [ -n "$TARGET" ]; then
    cd "$TARGET"
fi