---
name: ui-session
description: Keep the LiftPlanner app open as ONE persistent instance for UI automation, then drive it by objectName without relaunching it each time. Use when testing/driving the app across many steps (dump, find, click, set, invoke, screenshot). Pass a natural-language instruction or a raw driver command (e.g. "start", "stop", "status", "click startWorkoutButton", "find workout").
allowed-tools: Bash, Read
---

## Persistent LiftPlanner UI-automation session

Make sure the app is running once on its automation port, then drive it with bare
`liftplanner_driver.py <cmd>` calls. The app stays open between calls, so you stop
relaunching it for every step.

Two scripts, both under [tools/ui_automation/](../../../tools/ui_automation/), default port `49210`:

- [liftplanner_session.py](../../../tools/ui_automation/liftplanner_session.py) — lifecycle only:
  `start` / `status` / `stop` / `restart` / `logs`.
- [liftplanner_driver.py](../../../tools/ui_automation/liftplanner_driver.py) — sends one command
  to the running instance over TCP and prints the result.

> Do NOT use `liftplanner_driver.py shell` or `liftplanner_driver.py launch` for stepwise
> driving — those own the app and kill it when that one process exits, which is the
> relaunch-every-step problem this skill removes.

### Step 1 — always ensure the app is up first

```
python3 tools/ui_automation/liftplanner_session.py start
```

This checks port 49210 first. If the app already answers there — including an
instance you started by hand with `LIFTPLANNER_AUTOMATION_PORT=49210 ./build-desktop/src/liftplanner`
— it **adopts it and does nothing**. Otherwise it launches `build-desktop/src/liftplanner`
detached (`--binary PATH` / `--port N` to override) and waits until it responds. It
is idempotent, so it is safe to run before every driving session.

If it reports the binary is missing, build it first (or pass `--binary`).

### Step 2 — drive it with bare driver commands

One command per call; the app persists between them. Read each result from stdout.
All commands are prefixed with `python3 tools/ui_automation/liftplanner_driver.py`.

**Full command list:**

| Command | Syntax | Does |
|---|---|---|
| `ping` | `ping` | check the automation server answers (`pong`) |
| `dump` | `dump` | print the nested tree of *visible* named objects, with text/checked/value/currentText (hidden objects are omitted) |
| `find` | `find <substring>` | flat list of `objectName (class)` whose name contains the substring (empty = all) |
| `get` | `get <name> <property>` | read a property of the object with that objectName |
| `set` | `set <name> <property> <value>` | write a property (value is coerced to bool/int/float/str) |
| `click` | `click <name>` | click/press the control with that objectName |
| `tap` | `tap <x> <y>` | tap at window coordinates (use when there is no objectName) |
| `scroll` | `scroll <name> <dy> [dx]` | scroll a ListView/Flickable by `dy` px (positive = down); prints resulting `contentY` and `(top)`/`(bottom)` |
| `invoke` | `invoke <name> <method> [args…]` | call a `Q_INVOKABLE` method / slot on the object (args coerced) |
| `screenshot` | `screenshot <path>` | grab the window to a PNG file (use to confirm UI state) |
| `get_time` | `get_time` | read the app clock (`{dateTime, date, mocked}`) |
| `set_time` | `set_time <iso>` | pin the clock to an ISO date/datetime (installs a mock clock) |
| `advance_time` | `advance_time <days>` | move the mock clock forward by N days |
| `reset_time` | `reset_time` | restore the real system clock |

**Examples (each is one call):**

```
python3 tools/ui_automation/liftplanner_driver.py ping
python3 tools/ui_automation/liftplanner_driver.py dump
python3 tools/ui_automation/liftplanner_driver.py find startWorkout
python3 tools/ui_automation/liftplanner_driver.py click startWorkoutButton
python3 tools/ui_automation/liftplanner_driver.py tap 200 400
python3 tools/ui_automation/liftplanner_driver.py scroll workoutExerciseList 300
python3 tools/ui_automation/liftplanner_driver.py screenshot /tmp/liftplanner.png
python3 tools/ui_automation/liftplanner_driver.py set_time 2026-01-05
```

When unsure of a name, `dump` or `find <s>` first, then act on the discovered
`objectName`.

#### Discovering objectNames (key technique)

**Always use `find` and `dump` to discover controls — don't read the code.** This is how you'll learn what UI elements exist:

```bash
python3 tools/ui_automation/liftplanner_driver.py find ""           # list all objectNames
python3 tools/ui_automation/liftplanner_driver.py find "workout"     # search for "workout"-related names
python3 tools/ui_automation/liftplanner_driver.py dump               # full tree + properties
```

The main screens expose `objectName`s like `screenHome`, `screenWorkouts`,
`screenActiveWorkout`, `screenProfile`, and the bottom navigation buttons
(`nav*Button`). Switch screens by clicking the nav buttons, then `dump` to see
what the current screen exposes.

#### Reading state from `dump`

Most controls self-describe in `dump`, so you rarely need `get`:

- toggles/checkboxes/switches → `[checked]`/`[unchecked]`; spin boxes → `[value=N]`; combos → `[currentText='…']`.
- list rows surface their `text`; hidden/disabled controls are flagged `hidden`/`disabled`.

#### Important constraints

- **Don't take screenshots** unless the user explicitly asks for them.
- **Don't read code files** — use `dump` / `find` / `get` commands to explore the UI instead.
- **Only rely on script output** — if you can't discover something via the automation commands, ask the user.

### Step 3 — stop when finished (optional)

```
python3 tools/ui_automation/liftplanner_session.py stop
```

Leave it running if the user is mid-session or will keep testing. `stop` targets the
instance recorded by `start` (best-effort pid lookup for adopted ones).

### Determining the action from $ARGUMENTS

| Intent | Run |
|---|---|
| "start" / "open" / "uruchom" / "otwórz" | `liftplanner_session.py start` |
| "stop" / "close" / "zamknij" | `liftplanner_session.py stop` |
| "status" / "czy działa" | `liftplanner_session.py status` |
| "restart" / "zrestartuj" (e.g. after rebuild) | `liftplanner_session.py restart` |
| "logs" / "logi" | `liftplanner_session.py logs` |
| anything that looks like a driver command | `liftplanner_driver.py <cmd> <args…>` (run `start` first if not yet up) |

### Don't

- Don't use `liftplanner_driver.py shell` / `launch` for stepwise driving.
- Don't kill `liftplanner` by hand; use `stop`.
- Don't assume an objectName exists — verify with `find` / `dump` first.
- Don't read code files to understand the UI — discover via `dump` / `find` / `get` commands.
- Don't take screenshots unless the user explicitly asks for them.
- This is desktop-only (the automation server keys off `LIFTPLANNER_AUTOMATION_PORT`).
