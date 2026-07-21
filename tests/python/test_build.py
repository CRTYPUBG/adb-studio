from dataclasses import replace
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile

import build
from build_config import load_config


class BuildAutomationTests(unittest.TestCase):
    def test_safe_remove_tree_rejects_parent(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            parent = Path(directory)
            with self.assertRaises(build.BuildError):
                build.safe_remove_tree(parent, parent)

    def test_safe_remove_tree_removes_only_child(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            parent = Path(directory)
            child = parent / "generated"
            child.mkdir()
            build.safe_remove_tree(child, parent)
            self.assertFalse(child.exists())

    def test_release_notes_categorize_conventional_commits(self) -> None:
        commits = [("a1", "feat: dashboard"), ("b2", "fix: startup"),
                   ("c3", "refactor!: contracts")]
        with patch("build.commits_since_release", return_value=commits):
            notes = build.release_notes(load_config("Release"))
        self.assertIn("## Features", notes)
        self.assertIn("## Fixes and Closed Bugs", notes)
        self.assertIn("## Breaking Changes", notes)

    def test_signing_accepts_explicit_certificate_store_thumbprint(self) -> None:
        config = replace(load_config("Release"), sign_cert_path=None,
                         sign_cert_password=None,
                         sign_cert_thumbprint="49 07 F9 D3 33 1C 17 28 A8 2D 76 90 3F CD 9E 2E 20 E5 AE B5")
        self.assertEqual(build.signing_credentials(config),
                         ["/s", "My", "/sha1",
                          "4907F9D3331C1728A82D76903FCD9E2E20E5AEB5"])

    def test_signing_rejects_ambiguous_certificate_sources(self) -> None:
        config = replace(load_config("Release"), sign_cert_path=Path("certificate.pfx"),
                         sign_cert_password="secret",
                         sign_cert_thumbprint="4907F9D3331C1728A82D76903FCD9E2E20E5AEB5")
        with self.assertRaises(build.BuildError):
            build.signing_credentials(config)

    def test_portable_zip_uses_stable_product_root(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "temporary-staging-name"
            source.mkdir()
            (source / "adb-studio.exe").write_bytes(b"signed-binary")
            destination = root / "portable.zip"
            build.create_zip(source, destination, "ADB-Studio")
            with zipfile.ZipFile(destination) as archive:
                self.assertEqual(archive.namelist(), ["ADB-Studio/adb-studio.exe"])


if __name__ == "__main__":
    unittest.main()
