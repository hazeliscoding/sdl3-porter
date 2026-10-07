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
SDL = Path(__file__).resolve().parent / "sdl"
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


def hint_notes():
    """Returns a function that says what SDL3 did with each hint name a line quotes or sets.

    The bundled SDL_hints.h gives SDL3's hint strings, and the migration guide's
    SDL_hints.h section lists the hints SDL3 renamed and removed. A hint's string
    is its macro name with SDL_HINT_ replaced by SDL_.
    """
    try:
        header = (SDL / "include" / "SDL3" / "SDL_hints.h").read_text(encoding="utf-8")
        guide = (SDL / "docs" / "README-migration.md").read_text(encoding="utf-8")
    except OSError:
        return lambda line: []
    current = set(re.findall(r'^#define SDL_HINT_\w+\s+"(SDL_\w+)"', header, re.M))
    section = guide.split("\n## SDL_hints.h", 1)[-1].split("\n## ", 1)[0]
    renamed = dict(re.findall(r"^\* (SDL_HINT_\w+) => (SDL_HINT_\w+)", section, re.M))
    removed = {name: why for name, why in re.findall(r"^\* (SDL_HINT_\w+)(?: - (.*))?$", section, re.M)}

    def notes(line):
        found = []
        for name in dict.fromkeys(re.findall(r'"(SDL_[A-Z0-9_]+)"|\b(SDL_[A-Z0-9_]+)=', line)):
            name = name[0] or name[1]
            if name in current:
                continue
            macro = "SDL_HINT_" + name[len("SDL_"):]
            if macro in renamed:
                found.append(f'"{name}" is not an SDL3 hint: renamed, use {renamed[macro]}')
            elif macro in removed:
                why = f": {removed[macro]}" if removed[macro] else ""
                found.append(f'"{name}" is not an SDL3 hint: removed{why}')
        return found

    return notes


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

    hints = hint_notes()
    quiet = []
    matched = 0
    for name, title, patterns in sections(Path(args.references)):
        hits = []
        for f in files:
            for label, lines in (("", current[f]), (" (HEAD)", heads.get(f))):
                for number, line in enumerate(lines or [], 1):
                    if any(p.search(line) for p in patterns):
                        hits.append(f"{f.as_posix()}:{number}{label}: {line.strip()[:160]}")
                        if title == "hint-string-ignored":
                            hits.extend(f"    ^ {note}" for note in hints(line))
        if not hits:
            quiet.append(f"{name}: {title}")
            continue
        matched += 1
        print(f"== {name}: {title} ({len(hits)} hit{'s' if len(hits) != 1 else ''})")
        for hit in hits[:MAX_HITS]:
            print(hit)
        if len(hits) > MAX_HITS:
            print(f"... and {len(hits) - MAX_HITS} more; run this section's search for the rest")
        print()
    if quiet:
        print("No hits: " + "; ".join(quiet))
    if matched:
        print(f"\n{matched} section{'s' if matched != 1 else ''} with hits. Read each one in its file, and write a verdict "
              "for each before moving on: fixed, no change needed with the reason, or needs a human.")


if __name__ == "__main__":
    main()
