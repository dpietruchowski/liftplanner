---
name: commit
description: Stage and commit pending changes in the lift-planner repo using the project's Conventional Commits style. Use when the user wants to commit work in progress. Pass an optional one-line hint as argument to steer the message; otherwise infer from the diff.
allowed-tools: Bash(git status*), Bash(git diff*), Bash(git log*), Bash(git add*), Bash(git commit*), Bash(git restore --staged*)
---

## Commit pending changes

### Project commit-message style

This repo uses **Conventional Commits**. Look at recent history before drafting — the canonical pattern is:

```
<type>(<optional scope>): <subject>
```

Examples from `git log`:
- `feat: route runtime clock through TimeProvider for deterministic time`
- `refactor(ui): organize QML into thematic subdirectories and move icons to libs`
- `feat: run backend storage and services on a dedicated worker thread`
- `fix: data persistence bugs and refactor architecture for maintainability`
- `style: add outline button style and unify xSmall theme tokens`
- `chore: bump version to 1.0-rc4`

Conventions:
- **Type prefix is required.** In use here: `feat`, `fix`, `refactor`, `style`, `chore`.
- Optional scope in parentheses: `refactor(ui):`, `feat(workout):`.
- English, lowercase subject, imperative mood, no trailing period.
- One line is the norm; add a body only when the change genuinely needs explanation.
- **No** `Co-Authored-By` trailer — not used in this repo's history.

### Steps

1. Run in parallel: `git status`, `git diff` (unstaged), `git diff --cached` (staged), `git log -8 --oneline` (style refresher).
2. If nothing to commit (no staged + no untracked + no modified) — tell the user and stop.
3. Review the diff:
   - Watch for accidental inclusion of `*.db`, `*.keystore`, `build*/`, `*.user` — all gitignored, but warn if any slipped through.
   - Watch for committed credentials or absolute paths.
4. Stage files explicitly by name (never `git add -A` / `git add .`). If the user asked for a partial commit, stage only what matches their intent.
5. Draft the commit message using the style above. Pick the type from the change's nature (new behavior → `feat`, bug → `fix`, no-behavior restructuring → `refactor`, formatting/tokens → `style`, tooling/version → `chore`). If `$ARGUMENTS` is set, use it as steering input — but still rewrite into the project's style.
6. Show the user the proposed message + the file list, ask for confirmation before running `git commit`.
7. After commit: run `git status` to confirm a clean tree (or note remaining changes).

### Submodule handling

The repo has the `libs` submodule (see [.gitmodules](../../../.gitmodules)). If `git status` shows `libs` as `modified content` or `new commits`:

1. Commit **inside** `libs` first — `cd libs`, stage by name, commit in `libs`'s own style, `cd` back.
2. Then in root, stage the bumped `libs` pointer **together with** the other changes and commit. Mention the bump in the message.

If you commit root first, the submodule pointer still references the old SHA — always submodule-first. Do not touch `tests/third_party/googletest` unless it has real changes the user wants.

### Don't

- Do not commit `*.db`, `*.keystore`, build outputs, or anything matching `.gitignore`.
- Do not push. Committing only.
- Do not amend an existing commit unless the user explicitly asks.
- Do not use `--no-verify` to skip hooks.
- Do not add a `Co-Authored-By` trailer — it's not the convention here.
