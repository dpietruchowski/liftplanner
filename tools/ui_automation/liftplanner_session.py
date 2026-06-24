#!/usr/bin/env python3
"""Ensure ONE persistent LiftPlanner app instance is running for UI automation.

This is lifecycle only: it makes sure the app is up on a port (launching it
detached if needed), so you can then drive it with bare liftplanner_driver.py
commands across separate shell invocations without the app restarting each time.

Why: `liftplanner_driver.py --binary ... launch|shell` ties the app's lifetime to
that one process and kills it on exit. Here the app is launched detached with
LIFTPLANNER_AUTOMATION_PORT set; its TCP server keeps listening, and every later
`liftplanner_driver.py <cmd>` opens its own short-lived connection to it.

  liftplanner_session.py start [--binary PATH] [--port N]   # use running, else launch
  liftplanner_session.py status                             # running? pid / port
  liftplanner_session.py stop                               # kill the instance
  liftplanner_session.py restart [--binary PATH] [--port N]
  liftplanner_session.py logs [-n N]                        # tail the launched app log

`start` is idempotent: if the app already answers on the port (even one you
launched by hand with `LIFTPLANNER_AUTOMATION_PORT=49210 ./build-desktop/src/liftplanner`),
it adopts it and does nothing. Otherwise it launches `build-desktop/src/liftplanner`.

Drive the running instance with liftplanner_driver.py, e.g.:
  liftplanner_driver.py dump
  liftplanner_driver.py find startWorkout
  liftplanner_driver.py click startWorkoutButton
  liftplanner_driver.py scroll workoutExerciseList 300
  liftplanner_driver.py screenshot /tmp/liftplanner.png
"""

import argparse
import json
import os
import re
import signal
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from liftplanner_driver import (  # noqa: E402
    AutomationError,
    LiftPlannerDriver,
    DEFAULT_PORT,
    find_binary,
    runtime_env,
)

STATE_PATH = os.path.join(tempfile.gettempdir(), "liftplanner_ui_session.json")
LOG_PATH = os.path.join(tempfile.gettempdir(), "liftplanner_ui_session.log")


def _default_binary():
    """Prefer the desktop binary the project drives from; fall back to the rest."""
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.abspath(os.path.join(here, "..", ".."))
    desktop = os.path.join(root, "build-desktop", "src", "liftplanner")
    if os.path.isfile(desktop):
        return desktop
    return find_binary()


def _load_state():
    try:
        with open(STATE_PATH, "r", encoding="utf-8") as handle:
            return json.load(handle)
    except (OSError, ValueError):
        return None


def _save_state(state):
    with open(STATE_PATH, "w", encoding="utf-8") as handle:
        json.dump(state, handle)


def _clear_state():
    try:
        os.remove(STATE_PATH)
    except OSError:
        pass


def _pid_alive(pid):
    if not pid or pid < 0:
        return False
    try:
        os.kill(pid, 0)
    except OSError:
        return False
    return True


def _responds(port, timeout=1.0):
    driver = LiftPlannerDriver(port=port)
    try:
        driver.connect(timeout=timeout)
        return True
    except AutomationError:
        return False
    finally:
        driver.close()


def _pid_on_port(port):
    """Best-effort: pid of whatever is listening on 127.0.0.1:port (Linux ss)."""
    try:
        out = subprocess.run(
            ["ss", "-ltnp"], capture_output=True, text=True, timeout=3
        ).stdout
    except (OSError, subprocess.SubprocessError):
        return None
    for line in out.splitlines():
        if re.search(rf"127\.0\.0\.1:{port}\b", line):
            match = re.search(r"pid=(\d+)", line)
            if match:
                return int(match.group(1))
    return None


def cmd_start(opts):
    if _responds(opts.port):
        pid = _pid_on_port(opts.port)
        _save_state({"pid": pid, "port": opts.port, "log": LOG_PATH})
        where = f"pid {pid}" if pid else "unknown pid"
        print(f"already running on 127.0.0.1:{opts.port} ({where}); using it")
        return 0

    binary = opts.binary or _default_binary()
    if not binary or not os.path.isfile(binary):
        print(
            "error: app binary not found; build it or pass --binary "
            "(e.g. build-desktop/src/liftplanner)",
            file=sys.stderr,
        )
        return 1

    child_env = os.environ.copy()
    child_env.update(runtime_env(binary))
    child_env["LIFTPLANNER_AUTOMATION_PORT"] = str(opts.port)
    log = open(LOG_PATH, "wb")
    proc = subprocess.Popen(
        [binary],
        env=child_env,
        stdout=log,
        stderr=subprocess.STDOUT,
        stdin=subprocess.DEVNULL,
        start_new_session=True,
    )

    deadline = time.time() + opts.timeout
    while time.time() < deadline:
        if proc.poll() is not None:
            print(
                f"error: app exited early (code {proc.returncode}); see {LOG_PATH}",
                file=sys.stderr,
            )
            return 1
        if _responds(opts.port):
            _save_state({"pid": proc.pid, "port": opts.port, "log": LOG_PATH})
            print(f"started: pid {proc.pid} on 127.0.0.1:{opts.port} (log: {LOG_PATH})")
            return 0
        time.sleep(0.25)

    print(
        f"error: app did not become ready within {opts.timeout:g}s; see {LOG_PATH}",
        file=sys.stderr,
    )
    proc.terminate()
    return 1


def cmd_status(opts):
    state = _load_state()
    port = state.get("port") if state else DEFAULT_PORT
    if _responds(port):
        pid = (state or {}).get("pid") or _pid_on_port(port)
        print(f"running: pid {pid} on 127.0.0.1:{port}")
        return 0
    print("stopped")
    return 1


def cmd_stop(opts):
    state = _load_state()
    port = state.get("port") if state else DEFAULT_PORT
    pid = (state or {}).get("pid") or _pid_on_port(port)
    if not _pid_alive(pid):
        _clear_state()
        print("not running")
        return 0
    try:
        os.killpg(os.getpgid(pid), signal.SIGTERM)
    except OSError:
        try:
            os.kill(pid, signal.SIGTERM)
        except OSError:
            pass
    for _ in range(20):
        if not _pid_alive(pid):
            break
        time.sleep(0.25)
    if _pid_alive(pid):
        try:
            os.killpg(os.getpgid(pid), signal.SIGKILL)
        except OSError:
            try:
                os.kill(pid, signal.SIGKILL)
            except OSError:
                pass
    _clear_state()
    print(f"stopped: pid {pid}")
    return 0


def cmd_restart(opts):
    cmd_stop(opts)
    return cmd_start(opts)


def cmd_logs(opts):
    path = (_load_state() or {}).get("log", LOG_PATH)
    if not os.path.isfile(path):
        print(f"no log at {path}")
        return 1
    with open(path, "r", encoding="utf-8", errors="replace") as handle:
        lines = handle.readlines()
    for line in lines[-opts.lines :]:
        sys.stdout.write(line)
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Ensure a persistent LiftPlanner app instance for UI automation."
    )
    sub = parser.add_subparsers(dest="command", required=True)

    for name in ("start", "restart"):
        p = sub.add_parser(name)
        p.add_argument("--binary", help="path to liftplanner (default build-desktop)")
        p.add_argument("--port", type=int, default=DEFAULT_PORT)
        p.add_argument("--timeout", type=float, default=20.0)

    sub.add_parser("status")
    sub.add_parser("stop")

    p = sub.add_parser("logs")
    p.add_argument("-n", "--lines", type=int, default=40)

    opts = parser.parse_args(argv)

    handlers = {
        "start": cmd_start,
        "status": cmd_status,
        "stop": cmd_stop,
        "restart": cmd_restart,
        "logs": cmd_logs,
    }
    try:
        return handlers[opts.command](opts)
    except AutomationError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
