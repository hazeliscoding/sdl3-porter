"""Writes the eval score table into README.md from claude plugin eval results.

    python evals/readme_table.py <results.json> [<results.json> ...]

Each file is the --json output of one with-without run (the same schema as
aggregate-result.json). Several files can be combined, for suites run in batches
and for several models, as long as they used the same Claude Code version and
runs per case. The table, with a pair of columns per model, and a chart of the
same scores drawn as light and dark SVGs in docs/evals/, with a panel per model,
replace everything between the eval-table markers in README.md.
"""

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
README = ROOT / "README.md"
CHARTS = ROOT / "docs" / "evals"
START = "<!-- eval-table:start -->"
END = "<!-- eval-table:end -->"
WHOLE_PROGRAM = "port-sample"

# Models in the order the table and chart show them, with the names they use.
MODELS = {"claude-sonnet-5": "Sonnet 5", "claude-haiku-4-5-20251001": "Haiku 4.5"}

# GitHub's text colors and the brand accent, from AGENTS.md. "page" is the
# README background, which fills the hollow dots.
THEMES = {
    "scores.svg": {"ink": "#1f2328", "muted": "#59636e", "rule": "#d1d9e0", "accent": "#c2410c",
                   "without": "#8c959f", "page": "#ffffff"},
    "scores-dark.svg": {"ink": "#e6edf3", "muted": "#9198a1", "rule": "#3d444d", "accent": "#fb923c",
                        "without": "#6e7681", "page": "#0d1117"},
}
MONO = "ui-monospace, SFMono-Regular, Menlo, Consolas, monospace"
SANS = "-apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif"


def load(paths):
    """Returns ({model: {case name: case}}, Claude Code version, dates, runs per case)."""
    results, versions, dates, runs = {}, set(), set(), set()
    for path in paths:
        result = json.loads(Path(path).read_text(encoding="utf-8"))
        if result.get("partial"):
            sys.exit(f"{path} is partial: the run stopped early, so its scores are incomplete")
        if result["suite"].get("ablation") != "with-without":
            sys.exit(f"{path} has no without-skill arm: run it with --ablation with-without")
        model = result["suite"].get("modelOverride")
        if not model:
            sys.exit(f"{path} doesn't name its model: run it with --model")
        versions.add(result["claudeVersion"])
        dates.add(result["startedAt"][:10])
        cases = results.setdefault(model, {})
        for case in result["cases"]:
            if case["name"] in cases:
                sys.exit(f"{case['name']} appears more than once for {model}")
            cases[case["name"]] = case
            runs.add(len(case["arms"]["with"]))
    if len(versions) != 1:
        sys.exit(f"The results mix Claude Code versions {sorted(versions)}")
    if len(runs) != 1:
        sys.exit(f"Cases ran different numbers of times: {sorted(runs)}")
    return results, versions.pop(), sorted(dates), runs.pop()


def ordered_models(results):
    known = [m for m in MODELS if m in results]
    return known + sorted(m for m in results if m not in MODELS)


def ordered_cases(results):
    names = {name for cases in results.values() for name in cases}
    return sorted(n for n in names if n != WHOLE_PROGRAM) + ([WHOLE_PROGRAM] if WHOLE_PROGRAM in names else [])


def fired(cases):
    """How many runs with the skill loaded it, out of how many."""
    runs = [run for case in cases.values() for run in case["arms"]["with"]]
    hits = sum(1 for run in runs for g in run.get("graders", []) if g["name"] == "skill-fired" and g["passed"])
    return hits, len(runs)


def table(results, version, dates, runs):
    models = ordered_models(results)
    head = ["Case"]
    for model in models:
        head += [f"{MODELS.get(model, model)} with skill", "without"]
    lines = ["| " + " | ".join(head) + " |", "|" + "---|" * len(head)]
    for name in ordered_cases(results):
        label_ = "Whole program (`samples/bounce`)" if name == WHOLE_PROGRAM else f"`{name}`"
        cells = [label_]
        for model in models:
            case = results[model].get(name)
            if case:
                cells += [f"{case['aggregates']['score']:.2f}", f"{case['aggregates']['scoreWithout']:.2f}"]
            else:
                cells += ["–", "–"]
        lines.append("| " + " | ".join(cells) + " |")
    when = " to ".join([dates[0], dates[-1]] if len(dates) > 1 else dates)
    counts = [f"{hits} of {total} {MODELS.get(model, model)} runs"
              for model in models for hits, total in [fired(results[model])]]
    loaded = counts[0] if len(counts) == 1 else ", ".join(counts[:-1]) + " and " + counts[-1]
    ids = " and ".join(f"`{model}`" for model in models)
    lines += [
        "",
        f"{ids}, Claude Code {version}, {runs} run{'s' if runs != 1 else ''} per case with and without the skill, "
        f"{when}. The skill loaded in {loaded} with it installed. A score is the share of a case's graders that "
        "passed, not counting `skill-fired`, averaged over its runs. Generated by `evals/readme_table.py`.",
    ]
    return "\n".join(lines)


def chart(results, colors, footnote):
    """Dumbbell charts, one panel per model: a row per case, without and with the skill on a 0-1 axis."""
    left, right, width = 250, 700, 860
    ticks = (0, 0.25, 0.5, 0.75, 1)
    c = colors

    def x(score):
        return left + score * (right - left)

    out, y = [], 56
    for model in ordered_models(results):
        cases = results[model]
        out.append(f'<text x="16" y="{y}" font-family="{SANS}" font-size="15" font-weight="600" '
                   f'fill="{c["ink"]}">{MODELS.get(model, model)}</text>')
        out.append(f'<g font-family="{MONO}" font-size="11" fill="{c["muted"]}" text-anchor="middle">')
        for tick in ticks:
            text = {0: "0", 1: "1"}.get(tick, f"{tick:.2f}".lstrip("0"))
            out.append(f'<text x="{x(tick):.1f}" y="{y}">{text}</text>')
        out.append("</g>")
        y += 30
        groups = [
            ("The skill raised the score", [k for k in cases.values() if k["aggregates"]["delta"] > 0]),
            ("Same score either way", [k for k in cases.values() if k["aggregates"]["delta"] == 0]),
            ("The skill lowered the score", [k for k in cases.values() if k["aggregates"]["delta"] < 0]),
        ]
        rows = []
        for title, members in groups:
            if not members:
                continue
            rows.append(("group", title, y))
            y += 26
            for case in sorted(members, key=lambda k: (-k["aggregates"]["delta"], k["name"] == WHOLE_PROGRAM, k["name"])):
                rows.append(("case", case, y))
                y += 28
            y += 10
        top, bottom = rows[0][2] - 8, rows[-1][2] + 12
        for tick in ticks:
            out.append(f'<line x1="{x(tick):.1f}" y1="{top}" x2="{x(tick):.1f}" y2="{bottom}" '
                       f'stroke="{c["rule"]}" stroke-width="1"/>')
        for kind, item, ry in rows:
            if kind == "group":
                out.append(f'<text x="16" y="{ry + 4}" font-family="{SANS}" font-size="11" font-weight="600" '
                           f'letter-spacing="0.08em" fill="{c["muted"]}">{item.upper()}</text>')
                continue
            out += dumbbell(item, ry, x, right, width, c)
        y += 34
    height = y
    head = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}" '
        f'role="img" aria-label="Eval scores with and without sdl3-porter for each case">',
        f'<g font-family="{SANS}" font-size="13" fill="{c["muted"]}">',
        f'<circle cx="22" cy="20" r="7" fill="{c["accent"]}"/><text x="36" y="24">With sdl3-porter</text>',
        f'<circle cx="176" cy="20" r="6" fill="{c["page"]}" stroke="{c["without"]}" stroke-width="2.5"/>'
        f'<text x="190" y="24">Without it</text>',
        f'<text x="{width - 12}" y="24" text-anchor="end">Share of checks passed, averaged over runs</text>',
        "</g>",
    ]
    tail = [
        f'<text x="16" y="{height - 10}" font-family="{SANS}" font-size="11" fill="{c["muted"]}">{footnote}</text>',
        "</svg>",
    ]
    return "\n".join(head + out + tail) + "\n"


def dumbbell(case, y, x, right, width, c):
    totals = case["aggregates"]
    score, without, delta = totals["score"], totals["scoreWithout"], totals["delta"]
    out = [f'<text x="16" y="{y + 4}" font-family="{MONO}" font-size="13" fill="{c["ink"]}">{label(case["name"])}</text>']
    if score != without:
        low, high = sorted((score, without))
        out.append(f'<rect x="{x(low):.1f}" y="{y - 2}" width="{x(high) - x(low):.1f}" height="4" rx="2" '
                   f'fill="{c["accent"]}" fill-opacity="0.45"/>')
        out.append(f'<circle cx="{x(without):.1f}" cy="{y}" r="6" fill="{c["page"]}" '
                   f'stroke="{c["without"]}" stroke-width="2.5"/>')
    else:
        # Same score: a ring for the runs without the skill, around the dot for the runs with it.
        out.append(f'<circle cx="{x(score):.1f}" cy="{y}" r="10.5" fill="none" stroke="{c["without"]}" stroke-width="2.5"/>')
    out.append(f'<circle cx="{x(score):.1f}" cy="{y}" r="7" fill="{c["accent"]}"/>')
    out.append(f'<text x="{right + 28}" y="{y + 4}" font-family="{MONO}" font-size="12" fill="{c["muted"]}">'
               f'{without:.2f} → {score:.2f}</text>')
    sign = "+" if delta > 0 else "−" if delta < 0 else "±"
    tone, weight = (c["accent"], ' font-weight="700"') if delta > 0 else (c["muted"], "")
    out.append(f'<text x="{width - 12}" y="{y + 4}" font-family="{MONO}" font-size="12" text-anchor="end" '
               f'fill="{tone}"{weight}>{sign}{abs(delta):.2f}</text>')
    return out


def label(name):
    return "Whole program (bounce)" if name == WHOLE_PROGRAM else name


def picture():
    return (
        "<picture>\n"
        '  <source media="(prefers-color-scheme: dark)" srcset="docs/evals/scores-dark.svg">\n'
        '  <img alt="Eval scores with and without sdl3-porter for each case, as in the table below" '
        'src="docs/evals/scores.svg" width="860">\n'
        "</picture>\n"
    )


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    text = README.read_text(encoding="utf-8")
    if START not in text or END not in text:
        sys.exit(f"README.md has no {START} ... {END} block")
    before, rest = text.split(START, 1)
    _, after = rest.split(END, 1)
    results, version, dates, runs = load(sys.argv[1:])
    footnote = f"Claude Code {version}, {runs} runs per case with and without the skill"
    CHARTS.mkdir(parents=True, exist_ok=True)
    for name, colors in THEMES.items():
        (CHARTS / name).write_text(chart(results, colors, footnote), encoding="utf-8", newline="\n")
    new = f"{before}{START}\n{picture()}\n{table(results, version, dates, runs)}\n{END}{after}"
    README.write_text(new, encoding="utf-8", newline="\n")
    print(f"Updated the eval table and charts in {README} and {CHARTS}")


if __name__ == "__main__":
    main()
