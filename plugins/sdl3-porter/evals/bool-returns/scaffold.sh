#!/usr/bin/env bash
# Copies the bool-returns SDL2 sample into the run's working directory as a clean git repo.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cp -R "$here/../../../../samples/bool-returns/." .

git init -q
git add -A
git -c user.name=eval -c user.email=eval@example.invalid commit -q -m "SDL2 version"
