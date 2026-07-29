# AGENTS.md - pxt-mbit-more-v2

This document is a guide for AI agents (Antigravity, GitHub Copilot, etc.) to understand the `pxt-mbit-more-v2` project.

## Project Overview

**pxt-mbit-more-v2** is the micro:bit firmware and extension repository for Microbit More v2.
It includes MakeCode (PXT) TypeScript block definitions and C++ (CODAL / microbit-dal) custom BLE GATT service implementations.

- **Name**: pxt-mbit-more-v2
- **Version**: 0.3.0
- **Target**: micro:bit (v1 / v2)
- **Author**: Koji Yokokawa (@yokobond)

## Directory & Key File Structure

```
pxt-mbit-more-v2/
├── MbitMore.ts             # MakeCode TypeScript block definitions
├── MbitMore.cpp            # C++ bridge / helpers
├── MbitMoreDevice.cpp/.h   # Device control & sensor retrieval (C++)
├── MbitMoreService.cpp/.h  # BLE GATT service implementation (C++)
├── MbitMoreSerial.cpp/.h   # Serial communication handling (C++)
├── enums.d.ts / shims.d.ts # PXT type definitions & C++ shims
├── pxt.json                # PXT package configuration & dependencies
├── README.md               # Project documentation
└── _locales/               # Internationalization (ja / en)
```

## Tech Stack

- **Framework**: Microsoft MakeCode (PXT)
- **Languages**: TypeScript / C++ (CODAL / microbit-dal)
- **Communication**: Custom BLE Service (GATT) / USB Serial

## Toolchain Versions

The build target is pinned in `package.json`. Everything below follows from that single pin:

| Component | Version | Where it comes from |
|---|---|---|
| `pxt-microbit` (target) | **9.1.1** | `package.json` devDependencies — the only thing pinned by hand |
| `pxt-core` | 13.0.1 | `pxt-microbit@9.1.1` dependency |
| `pxt-common-packages` | 14.0.2 | `pxt-microbit@9.1.1` dependency |
| `codal-microbit-v2` (V2 / mbcodal) | v0.3.5 | `pxtarget.json` → `variants.mbcodal.compileService.codalTarget` |
| `microbit-v2-samples` | v0.2.13 | `pxtarget.json` → `variants.mbcodal.compileService.gittag` |
| `microbit` / microbit-dal (V1 / mbdal) | `yokobond/microbit#mbit-more` | `pxt.json` → `yotta.dependencies` (fork override) |
| Node.js | 20 | local + CI |

Both hardware variants are built on every `pxt build` (`alwaysMultiVariant: true`):
`mbdal` for micro:bit V1 and `mbcodal` for V2.

### Rules for bumping the toolchain

- **Change only the `pxt-microbit` line.** Never add `pxt-core` or `pxt-common-packages`
  to `devDependencies` — the target pins a matched pair, and overriding either one
  produces `INTERNAL ERROR: Non-thumb addr for pxt::decr` at hex-link time.
- **Always reinstall clean.** An in-place `npm install` keeps stale `pxt-core`
  resolutions in `package-lock.json` and reproduces the same error:
  ```bash
  rm -rf node_modules package-lock.json built pxt_modules && npm install
  ```
- **Verify the resolution** before building: `npm ls pxt-core` must show the top-level
  version matching `pxt-microbit`'s pin. Every `pxt` command also prints the target and
  `pxt-core` pair it is using.
- Details and diagnosis steps: `.agents/skills/pxt-build-troubleshooting/SKILL.md`

### V1 fork dependency

`pxt.json` overrides the V1 yotta dependency to `yokobond/microbit#mbit-more`, which in
turn points at `yokobond/microbit-dal#mbit-more`. That fork is 1 commit ahead of upstream
`microbit-dal@v2.2.0-rc6` and the whole diff is a single line — it changes
`MICROBIT_LIGHT_SENSOR_TICK_PERIOD` from 5 to 6. The V1 pins in `pxt-microbit` have not
moved across target upgrades, so this path is stable.

## Integration Architecture

```
┌─────────────────────────┐
│ Scratch VM (mbit-more-v2)│
└─────────────────────────┘
             │ Web Bluetooth / Scratch Link / Serial
             ▼
┌────────────────────────────────────────────────────────┐
│ micro:bit (pxt-mbit-more-v2)                            │
│ ├── MbitMoreService (GATT Service / Command Handler)  │
│ └── MbitMoreDevice  (Sensor / Pin / LED Control)     │
└────────────────────────────────────────────────────────┘
```

## Development & Testing Commands

```bash
# Install dependencies (use a clean reinstall after changing pxt-microbit)
npm install

# Run TypeScript Jest unit tests
npm test            # or `make test`

# Run Jest unit tests directly
npm run test:unit   # or `make test-unit`

# Regenerate shims.d.ts and enums.d.ts from C++ definitions
npm run build:shims # or `make test-shims`

# Run PXT tests
npm run test:pxt    # or `make test-pxt`

# Build (outputs to built/)
npm run build       # or `make build`
```

## Usage & Build

1. Open [MakeCode for micro:bit](https://makecode.microbit.org/)
2. Search and import `https://github.com/microbit-more/pxt-mbit-more-v2` via the **Extensions** menu
3. Download (flash) the program to your micro:bit

### Release artifact

`pxt build` writes to `built/`, which is gitignored:

| File | Contents |
|---|---|
| `built/mbdal-binary.hex` | micro:bit V1 image |
| `built/mbcodal-binary.hex` | micro:bit V2 image |
| `built/binary.hex` | combined universal hex (V1 + V2) |

`dist/binary.hex` is the only build output tracked in git — it is the distributed
firmware. Update it by copying the combined hex after the build has been smoke-tested on
real hardware:

```bash
cp built/binary.hex dist/binary.hex
```

### Releasing

Pushing a version tag (`0.2.6` or `v0.2.6`) makes the `MakeCode` workflow build the
firmware and attach `built/binary.hex` to that tag's release page as
`microbit-mbit-more-v2-<version>.hex`, with dots replaced by underscores — e.g.
`microbit-mbit-more-v2-0_2_6.hex`. This matches the naming of the manual releases up to
0.2.5. The release is created if it does not already exist (MakeCode normally creates it
when the extension is published), and re-cutting a tag overwrites the existing asset.

Release checklist:

1. Bump the version — `package.json` is the single source of truth:

   ```bash
   npm version patch --no-git-tag-version   # or minor / major
   ```

   The `version` lifecycle hook runs `scripts/sync-version.js`, which propagates the new
   number to `pxt.json`, to `MBIT_MORE_VERSION_STRING` in `MbitMoreCommon.h` (the string
   scrolled on the LED matrix at startup) and to the **Version** line above. Never edit
   those three by hand — `npm test` runs the same script with `--check` and fails CI on
   drift.

   `--no-git-tag-version` is deliberate: it skips npm's own commit and tag, because the
   release tag is cut on `master` after the merge (step 4) and npm would otherwise tag
   the current branch with a `v` prefix this repo does not use.

2. Rebuild and smoke-test on real hardware, then commit the distributed firmware:

   ```bash
   npm run build
   cp built/binary.hex dist/binary.hex
   ```

3. Commit on `develop`, then merge into `master` with `--no-ff`.
4. Tag `master` with the bare version (e.g. `0.2.6`, no `v`) and push the tag — CI builds
   and attaches the hex to the release.
5. Write the release notes (CI creates the release with an empty body).

## CODAL Resources

For C++ development targeting the micro:bit v2, the following resources are useful:
- [CODAL Documentation](https://lancaster-university.github.io/codal-documentation/index.html)
- [codal-microbit-v2 Repository](https://github.com/lancaster-university/codal-microbit-v2)
- [microbit-v2-samples Repository](https://github.com/lancaster-university/microbit-v2-samples)
- [Microbit v2 Unofficial Documentation (c272)](https://microbit.c272.org/)
