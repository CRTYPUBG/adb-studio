#!/usr/bin/env python3
import hashlib
import json
import pathlib
import sys


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    patterns = ("apps/studio/qml/**/*.qml", "assets/**/*.png", "assets/**/*.ico",
                "assets/**/*.svg")
    paths = sorted({path for pattern in patterns for path in root.glob(pattern)})
    resources = []
    for path in paths:
        relative = path.relative_to(root).as_posix()
        resource_type = "qml" if path.suffix == ".qml" else "icon" if path.suffix == ".ico" else "image"
        resources.append({
            "id": relative.removesuffix(".qml").replace("/", "."),
            "path": relative,
            "type": resource_type,
            "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
        })
    manifest = {"schemaVersion": 1, "resources": resources}
    output = root / "resources" / "res.crty"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
