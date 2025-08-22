#!/usr/bin/env sh
# run_in_screen.sh — run an app in a GNU screen session
set -eu

usage() {
  cat <<'USAGE'
Usage:
  run_in_screen.sh [-s SESSION_NAME] [--keep-open] [--log FILE] -- <app> [args...]

Options:
  -s, --session NAME   Name of the screen session (default: <app>-YYYYMMDD-HHMMSS)
  --keep-open          Keep the screen window open after the app exits (drops to a shell)
  --log FILE           Enable screen logging to FILE
  -h, --help           Show this help

Examples:
  ./run_in_screen.sh -- curl https://example.com
  ./run_in_screen.sh -s mybuild -- make -j8
  ./run_in_screen.sh --keep-open --log build.log -- ./build.sh --flag
Then attach with:
  screen -r <SESSION_NAME>
List sessions:
  screen -ls
USAGE
}

# ---- parse options
SESSION=""
KEEP_OPEN=0
LOGFILE=""

while [ "${1-}" ]; do
  case "$1" in
    -h|--help) usage; exit 0 ;;
    -s|--session)
      [ "${2-}" ] || { echo "Missing value for $1" >&2; exit 2; }
      SESSION=$2; shift 2 ;;
    --keep-open)
      KEEP_OPEN=1; shift ;;
    --log)
      [ "${2-}" ] || { echo "Missing value for $1" >&2; exit 2; }
      LOGFILE=$2; shift 2 ;;
    --)
      shift; break ;;
    -*)
      echo "Unknown option: $1" >&2; usage; exit 2 ;;
    *)
      break ;;
  esac
done

[ "${1-}" ] || { echo "Error: need an <app> to run." >&2; usage; exit 2; }

APP=$1; shift || true

# ---- prerequisites
command -v screen >/dev/null 2>&1 || {
  echo "Error: 'screen' is not installed or not in PATH." >&2
  exit 127
}

# Default session name
if [ -z "$SESSION" ]; then
  base=$(basename -- "$APP" 2>/dev/null || basename "$APP")
  SESSION="${base}-$(date +%Y%m%d-%H%M%S)"
fi

echo "Starting screen session: $SESSION"

# Default logfile name
if [ -z "$LOGFILE" ]; then
  LOGFILE="$SESSION.log"
fi

# Fail if session already exists
if screen -S "$SESSION" -Q select . >/dev/null 2>&1; then
  echo "Error: screen session '$SESSION' already exists. Use --session to pick another name." >&2
  exit 3
fi

# ---- run inside screen (detached)
if [ "$KEEP_OPEN" -eq 0 ]; then
  if [ -n "$LOGFILE" ]; then
    screen -DmS "$SESSION" -L -Logfile "$LOGFILE" -- "$APP" "$@"
  else
    screen -DmS "$SESSION" -- "$APP" "$@"
  fi
else
  # Keep window open after the app exits (drops you into a shell)
  if [ -n "$LOGFILE" ]; then
    screen -DmS "$SESSION" -L -Logfile "$LOGFILE" -- /bin/sh -c \
      'exec "$0" "$@"; code=$?; echo; echo "[process exited with code $code]"; exec /bin/sh' \
      "$APP" "$@"
  else
    screen -DmS "$SESSION" -- /bin/sh -c \
      'exec "$0" "$@"; code=$?; echo; echo "[process exited with code $code]"; exec /bin/sh' \
      "$APP" "$@"
  fi
fi

#echo "Started screen session: $SESSION"
#echo "Attach with: screen -r $SESSION"

