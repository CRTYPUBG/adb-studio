#!/usr/bin/env python3
import hashlib
import json
import pathlib
import sys


def fail(message: str) -> None:
    print(f"resource validation: {message}", file=sys.stderr)


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    manifest_path = root / "resources" / "res.crty"
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        fail(str(error))
        return 1

    entries = manifest.get("resources", [])
    ids = [entry.get("id") for entry in entries]
    paths = [entry.get("path") for entry in entries]
    errors = 0
    if len(ids) != len(set(ids)) or len(paths) != len(set(paths)):
        fail("duplicate resource id or path")
        errors += 1

    registered = set()
    for entry in entries:
        relative = pathlib.PurePosixPath(entry.get("path", ""))
        if relative.is_absolute() or ".." in relative.parts:
            fail(f"invalid path: {relative}")
            errors += 1
            continue
        path = root.joinpath(*relative.parts)
        registered.add(path.resolve())
        if not path.is_file():
            fail(f"missing resource: {relative}")
            errors += 1
            continue
        actual = hashlib.sha256(path.read_bytes()).hexdigest()
        if actual != entry.get("sha256"):
            fail(f"invalid hash: {relative}")
            errors += 1

    expected = {path.resolve() for path in (root / "apps" / "studio" / "qml").rglob("*.qml")}
    expected.update(path.resolve() for path in (root / "assets").rglob("*.png"))
    expected.update(path.resolve() for path in (root / "assets").rglob("*.ico"))
    expected.update(path.resolve() for path in (root / "assets").rglob("*.svg"))
    for path in sorted(expected - registered):
        fail(f"unregistered application resource: {path.relative_to(root)}")
        errors += 1
    for path in sorted(registered - expected):
        fail(f"unused or unknown resource: {path.relative_to(root)}")
        errors += 1
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
