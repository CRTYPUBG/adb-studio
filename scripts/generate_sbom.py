#!/usr/bin/env python3
import hashlib
import json
import pathlib
import sys
import uuid
from datetime import datetime, timezone


def digest(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    output = pathlib.Path(sys.argv[2] if len(sys.argv) > 2 else root / "build" / "sbom")
    output.mkdir(parents=True, exist_ok=True)
    version = json.loads((root / "version.crty").read_text(encoding="utf-8"))["version"]
    components = [
        {"name": "ADB Studio", "version": version, "license": "Apache-2.0"},
        {"name": "Qt", "version": "6.8.1", "license": "LGPL-3.0-only OR LicenseRef-Qt-Commercial"},
        {"name": "Microsoft Visual C++ Runtime", "version": "14.44",
         "license": "LicenseRef-Microsoft-Visual-Cpp-Redistributable"},
    ]
    packaged_root = root / "dist" / "ADB-Studio"
    packaged_files = (
        sorted(path for path in packaged_root.rglob("*") if path.is_file())
        if packaged_root.is_dir()
        else []
    )
    serial = f"urn:uuid:{uuid.uuid5(uuid.NAMESPACE_URL, f'adb-studio:{version}')}"
    cyclonedx = {
        "bomFormat": "CycloneDX",
        "specVersion": "1.6",
        "serialNumber": serial,
        "version": 1,
        "metadata": {"component": {
            "type": "application", "name": "ADB Studio", "version": version,
            "hashes": ([{"alg": "SHA-256", "content": digest(packaged_root / "adb-studio.exe")}]
                       if (packaged_root / "adb-studio.exe").is_file() else []),
        }},
        "components": [
            {"type": "library" if item["name"] != "ADB Studio" else "application",
             "name": item["name"], "version": item["version"],
             "licenses": [{"expression": item["license"]}]}
            for item in components
        ],
    }
    spdx = {
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": f"ADB-Studio-{version}",
        "documentNamespace": f"https://adb-studio.dev/sbom/{version}/{digest(root / 'version.crty')}",
        "creationInfo": {
            "creators": ["Tool: ADB-Studio-SBOM-Generator"],
            "created": datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z"),
        },
        "packages": [
            {"name": item["name"], "SPDXID": f"SPDXRef-{item['name'].replace(' ', '-')}",
             "versionInfo": item["version"], "downloadLocation": "NOASSERTION",
             "licenseConcluded": item["license"], "licenseDeclared": item["license"],
             "copyrightText": "NOASSERTION", "filesAnalyzed": False}
            for item in components
        ],
        "files": [
            {"fileName": path.relative_to(packaged_root).as_posix(),
             "SPDXID": f"SPDXRef-File-{index}",
             "checksums": [{"algorithm": "SHA256", "checksumValue": digest(path)}],
             "licenseConcluded": "NOASSERTION", "copyrightText": "NOASSERTION"}
            for index, path in enumerate(packaged_files, start=1)
        ],
    }
    (output / "bom.cdx.json").write_text(json.dumps(cyclonedx, indent=2) + "\n", encoding="utf-8")
    (output / "bom.spdx.json").write_text(json.dumps(spdx, indent=2) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
