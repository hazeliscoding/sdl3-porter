#!/usr/bin/env bash
# Copies the window-mode-null SDL2 sample into the run's working directory as a clean git repo.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cp -R "$here/../../../../samples/window-mode-null/." .

git init -q
git add -A
git -c user.name=eval -c user.email=eval@example.invalid commit -q -m "SDL2 version"
