#!/usr/bin/env python3
"""Generate firmware version headers from release tags (Python standard library only)."""
import argparse
from datetime import date
from pathlib import Path
import re
import subprocess

TAG = re.compile(r"v(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)\.(?:0|[1-9]\d*)(?:a\d+|b\d+|rc\d+|-[0-9A-Za-z.-]+)?(?:\+[0-9A-Za-z.-]+)?")
BASE_VERSION = "v1.0.0a0"

def git(source, *args):
    return subprocess.check_output(["git", "-C", str(source), *args], text=True, stderr=subprocess.PIPE).strip()

def git_version(source):
    if Path(git(source, "rev-parse", "--show-toplevel")).resolve() != source.resolve():
        raise ValueError("source directory has no Git metadata; use a version override")
    # Ignore unrelated tags, including upstream date-based releases.
    tags = [tag for tag in git(source, "tag", "--list").splitlines() if TAG.fullmatch(tag)]
    if tags:
        command = ["describe", "--tags", "--long", "--abbrev=7", "--dirty"]
        for tag in tags:
            command.extend(["--match", tag])
        try:
            description = git(source, *command)
        except subprocess.CalledProcessError:
            description = ""
        if description:
            match = re.fullmatch(r"(.+)-(\d+)-g([0-9a-f]+)(-dirty)?", description)
            if not match:
                raise ValueError("invalid Git description: " + description)
            tag, distance, revision, dirty = match.groups()
            if distance == "0" and not dirty:
                return tag
            return tag + ("." if "+" in tag else "+") + distance + ".g" + revision + (".dirty" if dirty else "")
    revision = git(source, "rev-parse", "--short=7", "HEAD")
    dirty = bool(git(source, "status", "--porcelain", "--untracked-files=no"))
    return BASE_VERSION + "+0.g" + revision + (".dirty" if dirty else "")

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--macro", choices=["GRBL_CORE_VERSION", "GRBL_RP2040_VERSION"], required=True)
    parser.add_argument("--version-override", default="")
    args = parser.parse_args()
    try:
        version = args.version_override or git_version(args.source_dir.resolve())
        if not TAG.fullmatch(version):
            raise ValueError("invalid semantic version: " + version)
        guard = args.macro + "_GENERATED_H"
        lines = ["#ifndef " + guard, "#define " + guard, f'#define {args.macro} "{version}"']
        if args.macro == "GRBL_RP2040_VERSION":
            lines.append(f'#define GRBL_BUILD_COMPILED "{date.today():%Y%m%d}_{version}"')
        lines += ["#endif", ""]
        text = "\n".join(lines)
        if not args.output.exists() or args.output.read_text() != text:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(text)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"Firmware version error: {error}\n")

if __name__ == "__main__":
    main()
