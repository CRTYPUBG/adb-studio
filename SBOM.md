# Software Bill of Materials

Every release produces SPDX JSON and CycloneDX JSON SBOMs for source, resolved dependencies and
packaged binaries. Artifacts include versions, hashes, licenses, supplier, download source and
dependency relationships. CI scans both documents for vulnerabilities and stores them with release
provenance. Missing, unversioned or unlicensed components block release.
