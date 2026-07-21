# Build and Release Automation

`build.py` is the only supported orchestration entry point. `build_config.py` loads public defaults
from `version.crty` and secret values from the process environment. The CLI supports `configure`,
`clean`, `build`, `rebuild`, `test`, `benchmark`, `lint`, `package`, `sign`, `release`, `publish` and
`nightly`; `--config` selects Debug or Release and `--version`/`--build-number` update the sole
version source before validation.

`package` produces unsigned developer-validation output. `sign`, `release`, `publish` and `nightly`
require exactly one explicit certificate source: PFX via `SIGN_CERT_PATH` plus
`SIGN_CERT_PASSWORD`, or the Windows `CurrentUser/My` store via `SIGN_CERT_THUMBPRINT`. Timestamp and
digest policy come from `SIGN_TIMESTAMP_URL` and `SIGN_HASH_ALGORITHM`. Commands never print the
password. First-party signing is allow-listed in `config/first-party-binaries.txt`.

Release order is intentionally fixed: validate, configure, build, test, analyze, deploy, sign
first-party deployed binaries, create Portable ZIP/Inno/MSIX/symbol packages, sign and verify the
installer and MSIX, benchmark, generate release notes/SBOM/license/evidence, then hash all artifacts.
`publish` is the only CLI command that pushes a tag and creates a GitHub Release.

The first signed release establishes `benchmarks/baseline.json`; later releases fail when the
versioned regression allowance or the absolute two-second startup budget is exceeded.
