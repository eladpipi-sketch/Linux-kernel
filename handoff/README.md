# Handoff: finish moving the 22-day course into the Claude Doc

Written on 13 November 2026 by the session that added Days 19–22. Everything
below is what that session meant to do but couldn't: the Claude Docs connector
returned HTTP 502 errors from api.anthropic.com from about 13:30 UTC on, then
disconnected, and `/mcp reconnect` isn't available in a cloud session.

**Prompt for the next session** (needs the Claude Docs connector; check that it
shows as connected at https://claude.ai/customize/connectors before starting):

> Read `handoff/README.md` on branch `ccr-77cd2602-bgjf5z` of my Linux-kernel
> repo and do what it says: bring my course doc in line with
> `handoff/course_22_days.md`, update its course map, export it as a PDF,
> replace `Linux_Internals_in_22_Days.pdf`, commit and push.

## Where things stand

- Branch `ccr-77cd2602-bgjf5z`. Commit `3e3aa92` added the Days 19–22 lab code,
  `setup/build-arm64.sh` and `setup/run-arm64.sh`, all tested (Days 19–21 in the
  QEMU VMs, Day 22 on the test machine's own 6.18 kernel). Commit `dd8381e`
  replaced the 18-day PDF with `Linux_Internals_in_22_Days.pdf` and updated
  `README.md`.
- That PDF already holds the whole 22-day course, but it was rendered locally
  from `course_22_days.md` with pandoc and Chromium, not exported from the doc,
  so its styling differs a little from earlier editions.
- The Claude Doc is still the 18-day version, at rev 149.

## What's left

1. Make the doc's text match `course_22_days.md` (this folder).
2. Replace the course map widget's code with `course_map_widget.jsx` (this
   folder) and set the map's caption to `the course by layer · 22 days`.
3. Export the doc as an A4 PDF, replace `Linux_Internals_in_22_Days.pdf` with
   it, delete this `handoff/` folder in the same commit, and push. End the
   commit message with the session's attribution lines.

## The doc

- Container: `{"kind": "project", "id": "5dd05fab-e935-4dd9-80c8-14079d034a65"}`
- Tab (file): `5388ccbf-5df1`. Root node: `1bf2caf3-393f`. Map widget node: `5c0024c2-b64a`.
- **Check the rev first.** It was 149 when this was written. If it's higher,
  the owner has edited the doc since: read it and merge those edits in, never
  overwrite them.

`course_22_days.md` is the doc's own markdown export at rev 149 with the
changes below applied, so exporting the live doc as markdown and diffing it
against this file shows exactly what to change. In document order:

1. Title: `Linux Internals in 18 Days` becomes `Linux Internals in 22 Days`.
2. Intro paragraph: "Eighteen sittings" becomes "Twenty-two sittings", and a
   sentence about Days 19–22 and the last chapters goes before "Every day ends
   with an exercise".
3. How this works: the "From Day 5 on" bullet now ends with Day 22's eBPF note;
   "That matters for the ESP32-S3 after Day 18" now says "in After Day 22";
   "The VM workflow makes Days 5–18 fast" becomes "Days 5–21".
4. Setup: "`day01/` to `day18/`" becomes "`day01/` to `day22/`".
5. The map: new widget code, caption `the course by layer · 22 days`.
6. Day 17's Notice: "see the ESP32-S3 in After Day 18" becomes "After Day 22".
7. New part after Day 18, before "Kernel rules for a C++ programmer":
   "Days 19–22: time, containers, an ARM board, eBPF", with Days 19, 20, 21
   and 22.
8. "After Day 18" becomes "After Day 22". In its "Embedded Linux" subsection,
   the last two sentences become one sentence pointing to Day 21 and Bootlin's
   training. The "Keep open while you work" subsection is removed; its
   content moved into Further reading.
9. Two new chapters before "Answer key": "Are you ready? From this course to a
   kernel job" and "Further reading".
10. Answer key: answers for Days 19–22 after Day 18's.
11. Sources and test environment: "Days 5–18 and every exercise answer"
    becomes "Days 5–18 and their exercise answers"; the Rust bullet says "After
    Day 22"; two new bullets (how Days 19–22 were tested, and that the book
    details were checked in November 2026); "What wasn't run" gains a sentence
    about Day 22's kprobe and fentry probes.

**How edits went through last time** (load the Docs guide's `topic.editing`
first; it's authoritative if it says otherwise):

- `update` with `ref {"object": "node", "id": "1bf2caf3-393f"}`, `engine
  "prose"` and `payload {"ops": [...]}`.
- Plain-text changes: `{"op": "replace", "target": {"kind": "find", "text":
  "<exact text>"}, "with": {"from": {"kind": "inline", "content": "<new
  text>"}, "as": "text"}}`. `nth` picks one occurrence when the text repeats.
  A find must stay inside one formatting run: it can't span bold, code or a
  link.
- Anything with formatting, and new sections: replace whole blocks with
  markdown, `{"op": "replace", "ifRev": <rev>, "ifHash": "<hash>", "target":
  {"kind": "blocks", "ids": [...]}, "with": {"as": "markdown", "from": {"kind":
  "inline", "content": "<markdown>"}}}`. Block ids and hashes come from `read`
  with the same ref and engine. To insert a section, replace the block it should
  follow with that block plus the new markdown.

## The map widget

The new code in `course_map_widget.jsx` keeps every existing anchor and text id.
It adds a second row to "Kernel core" (Days 19 and 20), a third bus driver (Day
21) and a fifth toolkit box (Day 22), and grows the viewBox from 760×688 to
760×776. Last time the widget changed through `update` with `ref {"object":
"node", "id": "5c0024c2-b64a"}`, `engine "widget"`, `payload {"kind":
"draft-edit", "edits": [{"quote": {"exact": "..."}, "replacement": "..."}]}`,
then `payload {"kind": "publish"}`; `read` with `payload {"kind":
"screenshot"}` shows the result. One edit whose quote runs from `return <svg` to
the end of the code, replaced by the same span of the new file, does it. If
the quote doesn't match, the widget has changed since; look at it first. The
caption is on the embed block in the doc, not in the widget.

## Export

`export` with the container, `file "5388ccbf-5df1"`, `format "pdf"` and
`paper "a4"`. The PDF comes back base64-encoded; a large result is saved to a
file, so decode that. The 18-day edition was 44 pages; with about 45% more text, expect about 60.

If Docs is still down and the PDF needs another local render: `pandoc -f gfm
-t html5` on `course_22_days.md` (with the map placeholder line replaced by
the widget drawn as a plain SVG), then Chromium's `page.pdf()` through
Playwright, A4, with "Linux Internals in 22 Days" in the header and "Page N of
M" in the footer.

## Never run or not verified

- Day 22's `kprobe:` and `fentry:` probes. The test machine's kernel had
  neither, so the day uses only tracepoints. Ubuntu's kernel should have both;
  run one there.
- KVM: every VM ran emulated. The speeds the doc expects under KVM weren't
  measured.
- A real ESP32-S3: the sketch was only compiled (arduino-cli, Espressif core
  3.3.12), and `usb_test` only ran against Day 17's software gadget.
- xv6 (After Day 22, optional): building downloaded code was blocked in the
  test environment, so those commands come from the original Part 1, unrun.
- Day 21 mentions `compatible = "gpio-leds"` as what a real board would use;
  that binding wasn't tried, because the arm64 config has no `LEDS_GPIO`.
- Links: the LFX mentorship wiki (https://wiki.linuxfoundation.org/lkmp)
  refused automated fetches (403), and Kernel Recipes (https://kernel-recipes.org/)
  returned 503 at check time. Search results confirm both exist. Every other
  link in the new chapters loaded.
