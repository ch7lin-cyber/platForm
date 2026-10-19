# CKLIB binary placeholder

`libcklib.a` is intentionally an empty, zero-byte placeholder. The physical CKLIB archive cannot be uploaded to this repository, so this file only preserves the expected library path.

The placeholder cannot satisfy firmware linking. For a target build, place the matching CKLIB archive in this directory on the build machine and configure the target linker to use it. Do not replace the placeholder in Git with the physical binary unless its distribution is explicitly approved.

Host unit tests use CKLIB test doubles and do not link this archive.
