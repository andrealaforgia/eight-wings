#!/bin/sh
# Re-runs the Round 5 ship redesign (needs the codex CLI).
cd "$(dirname "$0")/../.." || exit 1
cat docs/codex/BRIEF_COMMON.md docs/codex/BRIEF_ROUND2.md docs/codex/BRIEF_ROUND3.md docs/codex/BRIEF_ROUND4.md docs/codex/BRIEF_ROUND5.md | \
  /opt/homebrew/bin/codex exec -C . --sandbox workspace-write - > docs/codex/result_ship5.md 2>&1
