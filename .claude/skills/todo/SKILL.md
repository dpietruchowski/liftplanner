---
name: todo
description: Manage the prioritized project TODO list stored in .claude/TODO.md. Use when the user wants to add tasks, mark items done, view the list, or reorganize priorities. Pass a natural-language instruction as argument (e.g. "add: ekran profilu [high]", "done: ekran profilu", "show", "clear done").
allowed-tools: Read, Edit, Write
---

## Manage project TODO list

The list is stored in [.claude/TODO.md](../../../.claude/TODO.md). It is in Polish and divided into priority sections by emoji headers, plus a release-blockers part further down. Each item is a markdown checkbox (`- [ ]` or `- [x]`) and often carries `path:line` references into the code.

### File structure (respect it — do not rewrite into another format)

```markdown
# 📋 TODO — funkcjonalne

## 🔴 High priority
- [ ] ...

## 🟡 Medium priority
- [ ] ...

## 🔵 Low priority
- [ ] ...

## ❓ Do ustalenia
- [ ] ...

---

# 🚀 Blockery przed wydaniem na Google Play
## ⛔ Krytyczne ...
## 🟠 Wymagane porządki ...
## ⚖️ Licencje ...
## 📄 Po stronie sklepu ...
```

Keep these exact headers and ordering. The functional list and the release-blockers list are separate — add functional tasks to the priority sections, not the blockers part, unless the user clearly means a release blocker.

### Determining the action

Parse `$ARGUMENTS` (or the latest user message if `$ARGUMENTS` is empty) to determine intent:

| User intent | Action |
|---|---|
| Empty / "show" / "lista" / "co mamy" | Display the current list |
| "add: …" / "dodaj: …" / "nowe: …" | Add a new item |
| "done: …" / "zrobione: …" / "gotowe: …" / "skończyłem …" / "już jest …" | Mark matching item(s) as `[x]` |
| "undone: …" / "cofnij: …" | Mark matching item(s) back to `[ ]` |
| "remove: …" / "usuń: …" | Remove specific item |
| "clear done" / "wyczyść zrobione" / "usuń zrobione" | Remove all `[x]` items |
| "prioritize: … [high/medium/low]" / "przenieś: … [high/medium/low]" | Move item to a different priority section |

If intent is ambiguous, ask the user before modifying the file.

### Steps

1. **Read the file.** Use the Read tool on `.claude/TODO.md`. If it does not exist, create it with the structure above (Polish headers, empty sections).

2. **Execute the action:**

   **Show:** Print the contents formatted as markdown. Count open items per priority and show a summary line: `X otwartych (H high, M medium, L low), Y zrobionych` plus the blocker count if relevant.

   **Add:** Determine priority from keywords in the input:
   - `[high]` / `[h]` / `wysoki` / `pilne` / `ważne` → 🔴 High priority
   - `[low]` / `[l]` / `niski` / `kiedyś` / `nice to have` → 🔵 Low priority
   - default → 🟡 Medium priority

   Append the item as `- [ ] <opis>` under the correct section. Strip priority tags from the stored description. If the user supplied a `path:line` reference, keep it as a markdown link.

   **Mark done:** Find the item whose description best matches the input (case-insensitive substring match). Change `- [ ]` to `- [x]`. If multiple items match, list them and ask the user to confirm which one.

   **Mark undone:** Same matching logic, but change `- [x]` back to `- [ ]`.

   **Remove:** Find matching item(s), show them, ask for confirmation, then delete the line(s).

   **Clear done:** Remove all lines starting with `- [x]`. Ask for confirmation first.

   **Prioritize/move:** Find the item, remove it from its current section, append it to the target section.

3. **Write the result** with the Edit or Write tool (prefer Edit for small changes).

4. **Confirm** what was changed in one line, e.g.:
   `Oznaczono jako zrobione: "Ekran profilu" (było w 🔴 High priority)`
   `Dodano do 🟡 Medium priority: "Podgląd serii z poprzedniej sesji"`

### Don't

- Do not silently create a new item when trying to mark something done — if nothing matches, say so and ask.
- Do not reorder or reformat lines that were not touched.
- Do not remove the section headers or the `---` divider, even if a section is empty.
- Do not translate or rewrite existing items — keep their original Polish wording and `path:line` links.
- Do not commit the file — leave that to the user or the `commit` skill.
- Do not add timestamps, owners, or IDs unless the user asks.
