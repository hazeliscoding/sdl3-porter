#!/usr/bin/env python3
"""Runs every trap and guidance search in sdl3-porter's reference files over a
project, and prints the hits grouped by section.

    python3 find_traps.py PATH [PATH ...]

PATHs are the project's own source files or folders. Each section's "How to find
it" code block holds one regular expression per line. The script searches the
files as they are now and, for files that differ from git's HEAD, the HEAD
version too, marked "(HEAD)", because some traps only show in the SDL2 code.
A hit is a candidate, not a verdict: read it in context and decide.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

REFERENCES = Path(__file__).resolve().parent.parent / "references"
SOURCES = {".c", ".cc", ".cpp", ".cxx", ".c++", ".h", ".hh", ".hpp", ".hxx", ".inl", ".m", ".mm"}
MAX_HITS = 40


def sections(references):
    """Yields (file name, section title, [compiled patterns]) for every section with a search."""
    for md in sorted(references.glob("*.md")):
        text = md.read_text(encoding="utf-8")
        for part in re.split(r"^## ", text, flags=re.M)[1:]:
            title = part.splitlines()[0].strip()
            search = re.search(r"\*\*How to find it:\*\*(.*?)(?=\n\*\*Fix:\*\*|\Z)", part, re.S)
            if not search:
                continue
            blocks = re.findall(r"```[^\n]*\n(.*?)```", search.group(1), re.S)
            lines = [line.strip() for block in blocks for line in block.splitlines() if line.strip()]
            if lines:
                yield md.name, title, [re.compile(line) for line in lines]


def source_files(paths):
    for path in paths:
        path = Path(path)
        if path.is_file():
            yield path
        elif path.is_dir():
            for f in sorted(path.rglob("*")):
                if f.is_file() and f.suffix.lower() in SOURCES and ".git" not in f.parts:
                    yield f


def git(*args, cwd):
    try:
        done = subprocess.run(["git", *args], cwd=cwd, capture_output=True, text=True, encoding="utf-8", errors="replace")
    except OSError:
        return None
    return done.stdout if done.returncode == 0 else None


def head_versions(files):
    """Returns {file: lines at HEAD} for files git tracks that differ from HEAD."""
    if not files:
        return {}
    root = git("rev-parse", "--show-toplevel", cwd=files[0].resolve().parent)
    if not root:
        return {}
    root = Path(root.strip())
    changed = git("diff", "--name-only", "HEAD", cwd=root)
    if changed is None:
        return {}
    changed = {line.strip() for line in changed.splitlines() if line.strip()}
    result = {}
    for f in files:
        try:
            rel = f.resolve().relative_to(root).as_posix()
        except ValueError:
            continue
        if rel in changed:
            old = git("show", f"HEAD:{rel}", cwd=root)
            if old is not None:
                result[f] = old.splitlines()
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("paths", nargs="+", help="the project's source files or folders")
    parser.add_argument("--references", default=str(REFERENCES), help=argparse.SUPPRESS)
    args = parser.parse_args()

    files = list(source_files(args.paths))
    if not files:
        sys.exit("No C, C++ or Objective-C sources under " + " ".join(args.paths))
    current = {f: f.read_text(encoding="utf-8", errors="replace").splitlines() for f in files}
    heads = head_versions(files)

    quiet = []
    for name, title, patterns in sections(Path(args.references)):
        hits = []
        for f in files:
            for label, lines in (("", current[f]), (" (HEAD)", heads.get(f))):
                for number, line in enumerate(lines or [], 1):
                    if any(p.search(line) for p in patterns):
                        hits.append(f"{f.as_posix()}:{number}{label}: {line.strip()[:160]}")
        if not hits:
            quiet.append(f"{name}: {title}")
            continue
        print(f"== {name}: {title} ({len(hits)} hit{'s' if len(hits) != 1 else ''})")
        for hit in hits[:MAX_HITS]:
            print(hit)
        if len(hits) > MAX_HITS:
            print(f"... and {len(hits) - MAX_HITS} more; run this section's search for the rest")
        print()
    if quiet:
        print("No hits: " + "; ".join(quiet))


if __name__ == "__main__":
    main()
