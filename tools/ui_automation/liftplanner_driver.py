#!/usr/bin/env python3
"""Drive the LiftPlanner Qt/QML app over its in-process automation server.

The app must be built and run with the LIFTPLANNER_AUTOMATION_PORT env var set; this
script can do that for you (the `launch` / `shell` commands), or connect to an
instance you started yourself.

Why a server and not a standalone PySide script: the app is a compiled C++ Qt
binary. Its QObject/QML tree lives in that process's memory, so QObject.findChild
from a separate Python process cannot reach it. The app therefore exposes a tiny
localhost TCP server (UiAutomationServer) that walks the tree and dispatches
actions by objectName; this script is just its client.

Examples:
    # start the built app with automation enabled, then drop into a REPL
    ./liftplanner_driver.py --binary build-desktop/src/liftplanner shell

    # against an already-running instance (started with LIFTPLANNER_AUTOMATION_PORT=49210)
    ./liftplanner_driver.py dump
    ./liftplanner_driver.py find startWorkout
    ./liftplanner_driver.py click startWorkoutButton
    ./liftplanner_driver.py scroll workoutExerciseList 300
    ./liftplanner_driver.py screenshot /tmp/liftplanner.png

    # control the app clock (training history, planned workouts)
    ./liftplanner_driver.py set_time 2026-01-05
    ./liftplanner_driver.py advance_time 3
    ./liftplanner_driver.py reset_time
"""

import argparse
import json
import os
import shlex
import socket
import subprocess
import sys
import time

DEFAULT_PORT = 49210
DEFAULT_HOST = "127.0.0.1"


class AutomationError(RuntimeError):
    pass


def runtime_env(binary):
    """Env the binary needs to resolve its QML modules (Themed.Components, …).

    Mirrors the `qt-run` shell helper: the QML modules live under the build dir
    next to the executable, so QML_IMPORT_PATH / LD_LIBRARY_PATH must point there
    or QML loads fail with "<Type> is not a type".
    """
    build = os.path.dirname(os.path.dirname(os.path.abspath(binary)))
    lib_dirs = [
        os.path.join(build, "libs", "dbtoolkit"),
        os.path.join(build, "libs", "qmlcomponents"),
        os.path.join(build, "libs", "qmllive"),
        os.path.join(build, "Themed", "Components"),
    ]
    env = {}
    existing = os.environ.get("LD_LIBRARY_PATH", "")
    env["LD_LIBRARY_PATH"] = os.pathsep.join(lib_dirs + ([existing] if existing else []))
    env["QML_IMPORT_PATH"] = build
    env.setdefault("QT_QPA_PLATFORM", os.environ.get("QT_QPA_PLATFORM", "xcb"))
    return env


class LiftPlannerDriver:
    def __init__(self, host=DEFAULT_HOST, port=DEFAULT_PORT):
        self.host = host
        self.port = port
        self._sock = None
        self._stream = None
        self._proc = None

    # --- connection lifecycle -------------------------------------------------

    def launch(self, binary, env=None, timeout=20.0):
        """Spawn the built binary with automation enabled and connect to it."""
        child_env = os.environ.copy()
        child_env.update(runtime_env(binary))
        if env:
            child_env.update(env)
        child_env["LIFTPLANNER_AUTOMATION_PORT"] = str(self.port)
        self._proc = subprocess.Popen([binary], env=child_env)
        self.connect(timeout=timeout)
        return self

    def connect(self, timeout=10.0):
        """Connect to a running instance, waiting for the server to come up."""
        deadline = time.time() + timeout
        last_err = None
        while time.time() < deadline:
            if self._proc is not None and self._proc.poll() is not None:
                raise AutomationError(
                    f"app exited early with code {self._proc.returncode}"
                )
            try:
                self._sock = socket.create_connection((self.host, self.port), timeout=2.0)
                self._stream = self._sock.makefile("rwb")
                self.ping()
                return self
            except (OSError, AutomationError) as exc:
                last_err = exc
                self._close_socket()
                time.sleep(0.25)
        raise AutomationError(f"could not connect to {self.host}:{self.port}: {last_err}")

    def close(self):
        self._close_socket()

    def terminate(self):
        """Close the connection and stop the app we launched (if any)."""
        self._close_socket()
        if self._proc is not None and self._proc.poll() is None:
            self._proc.terminate()
            try:
                self._proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                self._proc.kill()
        self._proc = None

    def _close_socket(self):
        if self._stream is not None:
            try:
                self._stream.close()
            except OSError:
                pass
            self._stream = None
        if self._sock is not None:
            try:
                self._sock.close()
            except OSError:
                pass
            self._sock = None

    # --- low level rpc --------------------------------------------------------

    def _rpc(self, **request):
        if self._stream is None:
            raise AutomationError("not connected")
        line = (json.dumps(request) + "\n").encode("utf-8")
        self._stream.write(line)
        self._stream.flush()
        reply = self._stream.readline()
        if not reply:
            raise AutomationError("connection closed by app")
        response = json.loads(reply.decode("utf-8"))
        if not response.get("ok"):
            raise AutomationError(response.get("error", "unknown error"))
        return response

    # --- commands -------------------------------------------------------------

    def ping(self):
        return self._rpc(cmd="ping").get("pong", False)

    def dump(self):
        """Return the nested tree of named (objectName) objects."""
        return self._rpc(cmd="dump")["tree"]

    def find(self, substring=""):
        """Flat list of (objectName, class) whose name contains `substring`."""
        out = []

        def walk(nodes):
            for node in nodes:
                name = node.get("objectName", "")
                if substring in name:
                    out.append((name, node.get("class", "")))
                walk(node.get("children", []))

        walk(self.dump())
        return out

    def click(self, name):
        return self._rpc(cmd="click", name=name)

    def get(self, name, prop):
        return self._rpc(cmd="get", name=name, property=prop)["value"]

    # Properties that Qt Quick Controls only persist via interaction signals
    # (onToggled, onActivated, ...). Writing them with `set` changes the
    # property silently but the app never observes it, so the value reverts
    # on the next page load. Use `click` instead.
    _NOT_SETTABLE_PROPERTIES = {"checked", "currentIndex", "currentText"}

    def set(self, name, prop, value):
        if prop in self._NOT_SETTABLE_PROPERTIES:
            raise AutomationError(
                f"set {name} {prop} ... does not work: Qt Quick Controls only "
                f"apply '{prop}' changes via user interaction (onToggled/onActivated). "
                f"Use 'click' on the control instead."
            )
        return self._rpc(cmd="set", name=name, property=prop, value=value)

    def set_text(self, name, text):
        return self.set(name, "text", text)

    def invoke(self, name, method, *args):
        return self._rpc(cmd="invoke", name=name, method=method, args=list(args))

    def tap(self, x, y):
        return self._rpc(cmd="tap", x=x, y=y)

    def scroll(self, name, dy, dx=0):
        """Scroll a Flickable/ListView (found by name or its first flickable child).

        `dy` is in pixels: positive scrolls down, negative scrolls up; `dx`
        scrolls horizontally. Returns {contentY, contentX, atBeginning, atEnd}.
        """
        return self._rpc(cmd="scroll", name=name, dy=dy, dx=dx)

    def screenshot(self, path):
        return self._rpc(cmd="screenshot", path=os.path.abspath(path))

    def get_time(self):
        """Return {dateTime, date, mocked} for the app's current clock."""
        return self._rpc(cmd="get_time")

    def set_time(self, value):
        """Pin the clock to an absolute date or datetime (ISO 8601).

        A bare `YYYY-MM-DD` sets the date and keeps the time of day; a full
        `YYYY-MM-DDThh:mm:ss` sets both. Installs a mock clock on first use.
        """
        key = "dateTime" if "T" in value else "date"
        return self._rpc(cmd="set_time", **{key: value})

    def advance_time(self, days):
        """Move the (mock) clock forward by `days` days."""
        return self._rpc(cmd="advance_time", days=days)

    def reset_time(self):
        """Restore the real system clock."""
        return self._rpc(cmd="reset_time")


# --- pretty printing & CLI ----------------------------------------------------


def print_tree(nodes, indent=0):
    for node in nodes:
        name = node.get("objectName", "")
        cls = node.get("class", "")
        extra = []
        if "text" in node:
            extra.append(f'text={node["text"]!r}')
        if "currentText" in node:
            extra.append(f'currentText={node["currentText"]!r}')
        if "value" in node:
            extra.append(f'value={node["value"]}')
        if "checked" in node:
            extra.append("checked" if node["checked"] else "unchecked")
        if node.get("visible") is False:
            extra.append("hidden")
        if node.get("enabled") is False:
            extra.append("disabled")
        suffix = ("  [" + ", ".join(extra) + "]") if extra else ""
        print("  " * indent + f"{name} ({cls}){suffix}")
        print_tree(node.get("children", []), indent + 1)


def coerce(value):
    """Best-effort scalar coercion for CLI args (bool/int/float/str)."""
    lowered = value.lower()
    if lowered in ("true", "false"):
        return lowered == "true"
    for cast in (int, float):
        try:
            return cast(value)
        except ValueError:
            pass
    return value


def find_binary():
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.abspath(os.path.join(here, "..", ".."))
    candidates = [
        os.path.join(root, "build-desktop", "src", "liftplanner"),
        os.path.join(root, "build", "src", "liftplanner"),
        os.path.join(root, "cmake-build-debug", "src", "liftplanner"),
    ]
    for path in candidates:
        if os.path.isfile(path):
            return path
    return None


def run_shell(driver):
    print("Connected. Commands: dump | find <s> | click <name> | get <name> <prop> |")
    print("  set <name> <prop> <value> | invoke <name> <method> [args...] |")
    print("  tap <x> <y> | scroll <name> <dy> [dx] | screenshot <path> |")
    print("  get_time | set_time <iso> | advance_time <days> | reset_time | quit")
    while True:
        try:
            raw = input("liftplanner> ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break
        if not raw:
            continue
        parts = shlex.split(raw)
        cmd, args = parts[0], parts[1:]
        if cmd in ("quit", "exit"):
            break
        try:
            dispatch(driver, cmd, args)
        except AutomationError as exc:
            print(f"error: {exc}")
        except (TypeError, IndexError):
            print(f"bad arguments for {cmd}")


def dispatch(driver, cmd, args):
    if cmd == "dump":
        print_tree(driver.dump())
    elif cmd == "find":
        for name, cls in driver.find(args[0] if args else ""):
            print(f"{name} ({cls})")
    elif cmd == "click":
        driver.click(args[0])
        print("ok")
    elif cmd == "get":
        print(driver.get(args[0], args[1]))
    elif cmd == "set":
        driver.set(args[0], args[1], coerce(args[2]))
        print("ok")
    elif cmd == "invoke":
        driver.invoke(args[0], args[1], *[coerce(a) for a in args[2:]])
        print("ok")
    elif cmd == "tap":
        driver.tap(coerce(args[0]), coerce(args[1]))
        print("ok")
    elif cmd == "scroll":
        result = driver.scroll(args[0], coerce(args[1]), coerce(args[2]) if len(args) > 2 else 0)
        edge = " (top)" if result["atBeginning"] else (" (bottom)" if result["atEnd"] else "")
        print(f"contentY={result['contentY']:.0f}{edge}")
    elif cmd == "screenshot":
        driver.screenshot(args[0])
        print(f"saved {args[0]}")
    elif cmd == "get_time":
        info = driver.get_time()
        suffix = "" if info.get("mocked") else " (system clock)"
        print(f"{info['dateTime']}{suffix}")
    elif cmd == "set_time":
        print(driver.set_time(args[0])["dateTime"])
    elif cmd == "advance_time":
        print(driver.advance_time(coerce(args[0]))["dateTime"])
    elif cmd == "reset_time":
        print(driver.reset_time()["dateTime"])
    elif cmd == "ping":
        print("pong" if driver.ping() else "no")
    else:
        print(f"unknown command: {cmd}")


def main(argv=None):
    parser = argparse.ArgumentParser(description="Drive the LiftPlanner QML app by objectName.")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--binary", help="path to liftplanner; enables launching the app")
    parser.add_argument("command", nargs="?", default="shell")
    parser.add_argument("args", nargs="*")
    opts = parser.parse_args(argv)

    driver = LiftPlannerDriver(host=opts.host, port=opts.port)
    launched = False

    binary = opts.binary
    if opts.command in ("launch", "shell") and binary is None:
        binary = find_binary()

    try:
        if opts.command in ("launch", "shell") and binary:
            print(f"launching {binary} (LIFTPLANNER_AUTOMATION_PORT={opts.port}) ...")
            driver.launch(binary)
            launched = True
        else:
            driver.connect()

        if opts.command == "launch":
            print("app running with automation enabled. Ctrl-C to stop.")
            try:
                while driver._proc is None or driver._proc.poll() is None:
                    time.sleep(0.5)
            except KeyboardInterrupt:
                pass
        elif opts.command == "shell":
            run_shell(driver)
        else:
            dispatch(driver, opts.command, opts.args)
    except AutomationError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    finally:
        if launched:
            driver.terminate()
        else:
            driver.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
