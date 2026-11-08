#!/bin/sh
# Re-runs the three Round 3 Codex jobs in parallel (needs the codex CLI).
cd "$(dirname "$0")/../.." || exit 1
for j in A2 B2 C2; do
  ( cat docs/codex/BRIEF_COMMON.md docs/codex/BRIEF_ROUND2.md docs/codex/BRIEF_ROUND3.md docs/codex/JOB_$j.md | \
    codex exec -C . --sandbox workspace-write - > docs/codex/result_${j%2}3.md 2>&1 ) &
done
wait
