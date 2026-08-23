SELECTED_DIR=$(./cli_desktop)

if [ -n "$SELECTED_DIR" ]; then
      cd "$SELECTED_DIR"
      exec $SHELL
fi