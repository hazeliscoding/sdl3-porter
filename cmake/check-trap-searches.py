"""Checks that every trap card's search finds its own naive fixture, using the
skill's find_traps.py, so the one-command sweep can't miss a known trap.

    python check-trap-searches.py <find_traps.py> <references dir> <fixtures dir>
"""

import re
import subprocess
import sys
from pathlib import Path

script, references, fixtures = (Path(arg) for arg in sys.argv[1:4])
problems = []
for fixture in sorted(p for p in fixtures.iterdir() if p.is_dir()):
    trap = fixture.name
    done = subprocess.run(
        [sys.executable, str(script), "--references", str(references), str(fixture / "naive")],
        capture_output=True, text=True, encoding="utf-8",
    )
    if done.returncode != 0:
        problems.append(f"{trap}: find_traps.py failed: {done.stderr.strip()}")
    elif not re.search(rf"^== [\w.-]+: {re.escape(trap)} \(", done.stdout, re.M):
        problems.append(f"{trap}: its search finds nothing in fixtures/{trap}/naive")

if problems:
    sys.exit("Trap searches miss their own naive fixtures:\n  " + "\n  ".join(problems))
print(f"Every trap card's search finds its naive fixture ({len(list(fixtures.iterdir()))} traps)")
