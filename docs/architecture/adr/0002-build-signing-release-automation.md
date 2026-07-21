# ADR 0002: Build, signing and release automation

- Status: Accepted
- Date: 2026-07-20

## Context

Release construction, signing and publication must be reproducible locally and in GitHub Actions,
with one version source, no embedded secrets and no accidental signing of third-party binaries.

## Decision

Use lowercase `build.py` as the single command entry point and `build_config.py` as its
environment-backed configuration boundary. `version.crty` remains the sole version source.
Production release order is build/test/analyze, deploy, sign first-party binaries, create ZIP/Inno
Setup/MSIX packages, sign package executables, verify all signatures, then generate SBOMs, hashes,
manifests and evidence. Publishing is an explicit operation with Git tag and GitHub Release side
effects. Signing fails closed when certificate material is absent.

First-party signable basenames are allow-listed in `config/first-party-binaries.txt`. This prevents
Qt, MSVC runtime and other redistributed third-party files from being signed by CRTY.

## Consequences

Unsigned `package` artifacts support local validation, while `release`, `publish` and `nightly`
require Authenticode credentials. Windows SDK and Inno Setup are packaging prerequisites. Every
release produces portable, installer, MSIX, symbol, license, SBOM, checksum and audit evidence.

## 2026-07-21 Amendment

Local Windows signing may select an explicit private-key certificate from `CurrentUser/My` through
`SIGN_CERT_THUMBPRINT`. CI continues to use a temporary PFX decoded from GitHub Secrets. The two
certificate sources are mutually exclusive and automatic store selection is prohibited. Both paths
use SHA-256 Authenticode/RFC 3161 digests and mandatory `/pa /all` verification.
