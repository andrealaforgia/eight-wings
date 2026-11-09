#!/bin/sh
# Re-runs the two Round 6 Codex jobs in parallel (needs the codex CLI).
cd "$(dirname "$0")/../.." || exit 1
for j in E6 W6; do
  ( cat docs/codex/BRIEF_COMMON.md docs/codex/BRIEF_ROUND2.md docs/codex/BRIEF_ROUND3.md docs/codex/BRIEF_ROUND4.md docs/codex/BRIEF_ROUND6.md docs/codex/JOB_$j.md | \
    /opt/homebrew/bin/codex exec -C . --sandbox workspace-write - > docs/codex/result_${j}.md 2>&1 ) &
done
wait
