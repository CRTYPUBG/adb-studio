# Device Health Score

The score ranges from 0 to 100 and uses these weights:

| Category | Weight |
|---|---:|
| Connection | 25% |
| ADB | 20% |
| USB | 15% |
| Performance | 10% |
| Compatibility | 10% |
| Security | 10% |
| Recommended Settings | 10% |

`PASS` contributes 100, `WARNING` 60 and `FAIL` 0. `UNKNOWN` and `UNSUPPORTED` do not fabricate a
score; a neutral category value is used and the separate evidence-confidence percentage falls.
Recommendations remain visible so a low-confidence score cannot be mistaken for a verified healthy
device.
