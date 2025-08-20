#!/usr/bin/env sh
# run_app.sh — run an app with given argument(s)
set -eu

if [ "$#" -lt 2 ]; then
  echo "Usage: $0 <app> <arg1> [arg2 ...]" >&2
  exit 2
fi

APP=$1
shift

# Resolve the app: absolute/relative path or from PATH
if [ -x "$APP" ]; then
  CMD=$APP
else
  CMD=$(command -v -- "$APP" 2>/dev/null || true)
  if [ -z "${CMD:-}" ]; then
    echo "Error: '$APP' not found in PATH or not executable." >&2
    exit 127
  fi
fi

# Replace the shell with the target process, forwarding all args
exec "$CMD" "$@"
