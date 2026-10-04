"""Checks that every trap grader can fail: a port of each eval case's sample
that only ran SDL's rename scripts must fail every regex grader whose name
starts with "trap-" or "guidance-". Such a port still has every trap, so a
grader it passes can't tell a fixed trap from an unfixed one.

    python check-eval-graders.py <evals dir> <samples dir> <rename scripts dir> <work dir>

Graders use JavaScript regexes. The patterns stay within the syntax Python's
re module reads the same way.
"""

import re
import shutil
import subprocess
import sys
from pathlib import Path


def frontmatter(path):
    fields = {}
    for line in path.read_text(encoding="utf-8").split("---")[1].splitlines():
        key, sep, value = line.partition(":")
        if sep:
            fields[key.strip()] = value.strip()
    return fields


def rename_only_port(sample, scripts, port):
    shutil.rmtree(port, ignore_errors=True)
    shutil.copytree(sample, port)
    # The order the skill uses: headers, then symbols, then macros.
    for script, args in (("rename_headers.py", []), ("rename_symbols.py", ["--all-symbols"]), ("rename_macros.py", [])):
        subprocess.run([sys.executable, str(scripts / script), *args, str(port / "src")],
                       check=True, stdout=subprocess.DEVNULL)


def main():
    evals, samples, scripts, work = (Path(arg) for arg in sys.argv[1:5])
    problems = []
    checked = 0

    for case in sorted(evals.glob("*/case.yaml")):
        case_dir = case.parent
        scaffold = case_dir / "scaffold.sh"
        match = re.search(r"samples/([\w-]+)/", scaffold.read_text(encoding="utf-8")) if scaffold.exists() else None
        if not match:
            continue
        port = work / case_dir.name
        rename_only_port(samples / match.group(1), scripts, port)

        for grader in sorted((case_dir / "graders").glob("*.md")):
            if not grader.stem.startswith(("trap-", "guidance-")):
                continue
            fields = frontmatter(grader)
            if fields.get("type") != "regex":
                problems.append(f"{case_dir.name}/{grader.name} isn't a regex grader")
                continue
            path = re.search(r'path:\s*"([^"]+)"', fields["target"]).group(1)
            pattern = fields["pattern"][1:-1].replace("''", "'")
            found = re.search(pattern, (port / path).read_text(encoding="utf-8")) is not None
            passed = found if fields.get("match", "contains") == "contains" else not found
            if passed:
                problems.append(f"{case_dir.name}/{grader.name} passes on a port that only ran the rename scripts")
            checked += 1

    if problems:
        sys.exit("Trap graders that can't fail:\n  " + "\n  ".join(problems))
    print(f"{checked} trap graders fail on ports that only ran the rename scripts")


if __name__ == "__main__":
    main()
