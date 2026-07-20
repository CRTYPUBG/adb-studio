# Windows Artifact Signing

Configure `WINDOWS_CERTIFICATE_BASE64` and `WINDOWS_CERTIFICATE_PASSWORD` as GitHub Actions secrets.
Decode the certificate only into the runner temporary directory, invoke `signtool.exe` with SHA-256
file/timestamp digests and the organization timestamp server, verify every signature, then delete the
temporary certificate. Never upload the PFX or print secret values. Environment protection should
require maintainer approval for production signing.
