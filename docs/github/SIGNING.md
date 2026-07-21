# Windows Artifact Signing

Configure `SIGN_CERT_BASE64` and `SIGN_CERT_PASSWORD` as GitHub Actions secrets.
Decode the certificate only into the runner temporary directory, invoke `signtool.exe` with SHA-256
file/timestamp digests and the organization timestamp server, verify every signature, then delete the
temporary certificate. Never upload the PFX or print secret values. Environment protection should
require maintainer approval for production signing.

GitHub Secrets are `SIGN_CERT_BASE64` and `SIGN_CERT_PASSWORD`. Repository variables are
`SIGN_TIMESTAMP_URL` and `MSIX_PUBLISHER`. Workflows decode the PFX only under `RUNNER_TEMP`, expose
its path as `SIGN_CERT_PATH`, and delete it in an `always()` cleanup step.

Local Windows builds may instead set `SIGN_CERT_THUMBPRINT` to the 40-hexadecimal SHA-1 thumbprint
of an explicitly selected private-key certificate in `CurrentUser/My`. `SIGN_CERT_PATH` and
`SIGN_CERT_THUMBPRINT` are mutually exclusive. Store-backed signing does not require or read a PFX
password. Never select a certificate automatically when multiple code-signing certificates exist.

Use the certificate subject as `MSIX_PUBLISHER`; the values must match exactly. DigiCert's RFC 3161
endpoint is `http://timestamp.digicert.com`. Although its transport endpoint is HTTP, the returned
timestamp token is cryptographically signed and is verified by `signtool`. SHA-256 is required for
both file and timestamp digests. `config/first-party-binaries.txt` is the reviewed allow-list; never
add Qt, MSVC, scrcpy, ADB or other third-party files.
