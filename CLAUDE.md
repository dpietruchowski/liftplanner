# LiftPlanner

## Scratch space

Use `tmp/` in the repo root for everything throwaway: helper scripts, logs, gdb
output, copies of the database, offscreen run dirs. It is gitignored.

Do not use `/tmp` or the session scratchpad — `tmp/` keeps this project's
working files next to the code and survives across sessions.

## Never touch the real user data

Test and debug runs must not mutate:

- `liftplanner.db` in the repo root
- `~/.local/share/LiftPlanner/`

Copy the database into `tmp/` and redirect `XDG_DATA_HOME` there:

```sh
mkdir -p tmp/run && cp liftplanner.db tmp/run/
cd tmp/run && XDG_DATA_HOME=$PWD/data QT_QPA_PLATFORM=offscreen \
  QML2_IMPORT_PATH=../../build-desktop:../../build-desktop/src \
  ../../build-desktop/src/liftplanner
```

## Database access

All DDL and DML goes through the dbtoolkit query builders. No raw SQL.

## Code style

No comments in code — write it so it explains itself. Run the `format` CMake
target (or the `format-code` skill) before committing.

## QML

`src/CMakeLists.txt` globs the QML sources, so **a new `.qml` or `.cpp` file
needs a CMake reconfigure**, not just a rebuild:

```sh
cmake -S . -B build-desktop
```

`qmlcachegen` catches syntax and some type errors at build time, but not wrong
property names or bad bindings — those only show up at runtime.

## Commits

Conventional Commits, English, imperative, lowercase subject. No
`Co-Authored-By` trailer. Stage files explicitly by name; never `git add -A`.
Do not push unless asked.
