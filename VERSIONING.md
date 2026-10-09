# Firmware versions

Core and RP2040 have independent Git release tags. Start each repository at
`v1.0.0a0`. Subsequent stable releases use `vMAJOR.MINOR.PATCH`: increment major
for incompatible changes, minor for compatible features, and patch for fixes.
Alpha tags such as `v1.0.0a0` are also accepted.

Commit and tag the core release first, then update and commit the RP2040 `grbl`
submodule pointer before tagging RP2040. Tags must identify the committed release
sources; they cannot include uncommitted edits. Fetch tags in both repositories
(including the submodule) before building.

An exact clean tag reports `v1.0.0a0`. One later commit reports
`v1.0.0a0+1.gabcdef0`; tracked local changes append `.dirty`. Without any reachable
release tag the initial baseline is `v1.0.0a0+0.g<hash>`. The generators run on
every build, so tagging or switching commits does not require deleting the build
folder. Headers are rewritten only when their contents change.

The RP2040 welcome string keeps the protocol identifier and build date:
`MrblHAL 1.1f_20261009_v1.0.0a0 [BB:100,RX:1024,MRBCHK:1] ...`

`$I` retains the existing `[VER:1.1f.20261004:...]` record and adds:

```
[CORE:v1.0.0a0]
[RP2040:v1.0.0a0]
```

The core record is emitted when the core CMake target is used; the RP2040 record
is emitted by RP2040 builds. The bootloader maintains its own version.

For source archives without Git metadata, supply both versions explicitly:

```
cmake .. -DGRBL_CORE_VERSION_OVERRIDE=v1.0.0a0 -DGRBL_RP2040_VERSION_OVERRIDE=v1.0.0a0
make -j16
```

These overrides bypass Git state detection and should describe the actual sources.
Normal checkout builds need only `cmake ..` and `make -j16`. Python 3 is required;
no third-party Python packages are used. Plugin firmware filenames and expected
update versions must use the new complete welcome version when publishing firmware.
