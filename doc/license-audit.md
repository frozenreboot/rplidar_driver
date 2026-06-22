# License Audit

This document records the license review performed before the first rosdistro
release of this package. It covers the package's own license declaration, the
license of vendored upstream SDK code, components that were intentionally
excluded, and the verification commands used to reach each conclusion.

## Package metadata

* Package name: `rplidar_driver` (declared in `package.xml`)
* Version: `1.3.1`
* Declared license: `BSD-2-Clause` (a valid SPDX identifier)

## Package license

This package declares `BSD-2-Clause` in `package.xml`.

The full license text is provided in the top-level `LICENSE` file. That file
reproduces the BSD-2-Clause terms together with the copyright notices of all
relevant parties (see below), which is the instrument that covers both the
first-party code and the vendored upstream code.

## Package-maintained (first-party) code

The ROS 2 package code maintained in this repository is distributed under
`BSD-2-Clause`. This includes:

- ROS 2 lifecycle node implementation
- driver wrapper layer
- standalone entry point
- launch files
- parameter files
- tests
- package metadata
- documentation maintained in this repository

First-party source files carry a copyright header attributing
`frozenreboot` (2025-2026) alongside the upstream RoboPeak / Slamtec
copyrights, and note that the project is a refactored derivative of the
original `rplidar_ros` package. The binding license grant for these files is
the top-level `LICENSE`.

## Vendored upstream SDK code

This repository vendors selected source files from the RoboPeak / Slamtec
RPLIDAR SDK under `src/sdk`. Only the SDK/library code required for RPLIDAR
communication is included; no upstream demo or application code is vendored.

### Upstream SDK provenance

The vendored SDK source under `src/sdk` was imported from the `sdk/`
subdirectory of the upstream `Slamtec/rplidar_ros` repository.

* Upstream repository: `Slamtec/rplidar_ros`
* Upstream commit: `24cc9b6dea97e045bda1408eaa867ce730fd3fc3`
* Upstream path: `sdk/`
* Vendored path in this repository: `src/sdk/`
* Import method: selected SDK/library source files were copied into this repository
* Imported components: SDK/library code required for RPLIDAR communication
* Excluded components: upstream ROS node code, demo/sample applications, GUI tools, binary blobs, and platform-specific binary drivers
* Local modifications: none to the vendored SDK files (verified byte-identical to upstream at the above commit); integration is confined to this package's own wrapper and build files

The vendored SDK code carries copyright notices from:

* RoboPeak Team / RoboPeak Project
* Shanghai Slamtec Co., Ltd.

The SDK header dates observed in the vendored files span roughly 2009-2014
(RoboPeak) and 2014-2022 (Slamtec). The copied SDK/library files carry
BSD-2-Clause-style license headers or upstream RoboPeak / Slamtec copyright
notices. Core SDK source files such as `src/sdk/src/rplidar_driver.cpp` and
`src/sdk/src/sl_lidar_driver.cpp` carry the full BSD-2-Clause grant text in
their file headers.

## Excluded upstream components

The following upstream SDK components are **not** included in this ROS package
release, and were confirmed absent from the current tracked source tree (the
tree captured at the release tag is what bloom packages):

- demo / sample applications
- GUI / frame-grabber applications
- vendor binary blobs
- platform-specific binary drivers
- archive / binary files such as `.zip`, `.rar`, `.dll`, `.so`, `.a`

## Copyleft (GPL-family) license check

A broadened scan was used so that spelled-out license grants (which do not
contain the literal substring `gpl`) are also caught:

```bash
git grep -nEi "gpl|lgpl|agpl|general public license|free software foundation|copyleft|gnu" -- .
```

No GPL, LGPL, AGPL, or other copyleft license identifiers - whether as
acronyms or spelled out - were found anywhere in the repository.

## Binary artifacts check

```bash
git ls-files | grep -iE '\.(so|a|dll|lib|zip|rar|exe|bin|o|out)$'
git ls-files -z | xargs -0 file | grep -vi text
```

No **vendor** binary blobs are present. The only tracked binary artifact is
`doc/architecture.png`, which is a first-party documentation diagram produced
for this repository, not a redistributed third-party binary.

## Legacy SDK header note

Some legacy RoboPeak HAL / arch utility files contain short copyright-only
headers rather than a full per-file license grant. Examples include the
byte-order, socket, and waiter helpers under `src/sdk/src/hal` and
`src/sdk/src/arch/linux`.

These files are part of the upstream RoboPeak / Slamtec SDK source tree and
are treated as SDK-derived code. Their original copyright notices are
preserved, and the top-level `LICENSE` records the BSD-2-Clause terms together
with the RoboPeak / Slamtec copyright notices used by the vendored SDK code.

## External ROS dependencies

External ROS dependencies such as `rclcpp`, `rclcpp_lifecycle`,
`sensor_msgs`, `diagnostic_updater`, `tf2`, and `launch_ros` are referenced as
package dependencies in `package.xml` and are **not** vendored into this
repository.

## Future work (non-blocking)

* Per-file SPDX identifiers / REUSE compliance could be added in a later
  release to make automated license scanning fully self-describing.

## Conclusion

Based on this review:

* `package.xml` declares `BSD-2-Clause`, a valid SPDX identifier
* the top-level `LICENSE` file is present and carries both the first-party and
  upstream RoboPeak / Slamtec copyright notices under BSD-2-Clause
* core upstream SDK files carry the full BSD-2-Clause grant in their headers
* legacy copyright-only SDK headers are documented and covered by the
  top-level `LICENSE`
* no GPL / LGPL / AGPL / copyleft identifiers were detected (broadened scan)
* no upstream demo / application code is present in the current tracked source tree
* no vendor binary blobs are present (the sole binary artifact is a
  first-party documentation image)

No release-blocking licensing issue was identified for the first rosdistro
release based on the checks documented above.
