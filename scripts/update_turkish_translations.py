#!/usr/bin/env python3
"""Populate unfinished Qt TS messages using the configured translation provider."""

from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import pathlib
import re
import sys
import time
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ElementTree


PLACEHOLDER = re.compile(r"%\d+|%n")


def translate(source: str) -> str:
    protected = source
    tokens: dict[str, str] = {}
    for index, value in enumerate(PLACEHOLDER.findall(source)):
        token = f"ZXQPH{index}QXZ"
        protected = protected.replace(value, token, 1)
        tokens[token] = value
    query = urllib.parse.urlencode(
        {"client": "gtx", "sl": "en", "tl": "tr", "dt": "t", "q": protected}
    )
    request = urllib.request.Request(
        "https://translate.googleapis.com/translate_a/single?" + query,
        headers={"User-Agent": "ADB-Studio-Translation-Updater/1.0"},
    )
    for attempt in range(3):
        try:
            with urllib.request.urlopen(request, timeout=20) as response:
                payload = json.loads(response.read().decode("utf-8"))
            result = "".join(part[0] for part in payload[0] if part[0])
            for token, value in tokens.items():
                result = result.replace(token, value)
            return result
        except Exception:
            if attempt == 2:
                raise
            time.sleep(1 + attempt)
    raise RuntimeError("translation provider did not return a result")


def main() -> int:
    path = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "translations/adb_studio_tr.ts")
    tree = ElementTree.parse(path)
    pending: dict[str, list[ElementTree.Element]] = {}
    for message in tree.findall(".//message"):
        source = message.findtext("source", default="")
        translation = message.find("translation")
        if translation is None:
            translation = ElementTree.SubElement(message, "translation")
        if not translation.text or translation.get("type") == "unfinished":
            pending.setdefault(source, []).append(translation)

    results: dict[str, str] = {}
    with ThreadPoolExecutor(max_workers=8) as pool:
        futures = {pool.submit(translate, source): source for source in pending}
        for future in as_completed(futures):
            source = futures[future]
            results[source] = future.result()

    for source, elements in pending.items():
        for translation in elements:
            translation.text = results[source]
            translation.attrib.pop("type", None)
    ElementTree.indent(tree, space="  ")
    tree.write(path, encoding="utf-8", xml_declaration=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
