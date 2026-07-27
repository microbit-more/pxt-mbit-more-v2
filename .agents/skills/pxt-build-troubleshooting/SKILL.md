---
name: pxt-build-troubleshooting
description: Use when pxt test or pxt build fails with an internal compiler error like "Non-thumb addr for pxt::decr" in a MakeCode extension project.
---

# PXT Build Troubleshooting

When running `pxt test` or `pxt build`, you might hit an internal compiler error during the hex-linking phase:

```
INTERNAL ERROR: Error: Non-thumb addr for pxt::decr / A0380300
    at Object.oops (.../pxt.js)
    at readPointers (.../pxt.js)
    at Object.setupFor (.../pxt.js)
```

## Cause

**This is a toolchain version-skew problem, not an API-compatibility problem.** Do not
assume your C++ is too old for the target.

The error string comes from `readPointers()` in `pxtcompiler/emitter/hexfile.ts`
(microsoft/pxt). PXT scans every `.cpp`/`.h` carrying a `//%` annotation — across the
DAL/CODAL, `pxt-common-packages` and your extension — and builds an ordered function
table. It then walks the jump table embedded in the base hex and asserts every resolved
address has the Thumb bit set. If the shim table and the base hex's jump table are out of
sync, an address comes back even and you get this error.

`pxt::incr` / `pxt::decr` are red herrings. They are declared with `//%` in
`pxt-common-packages/libs/base/pxtbase.h` and so appear in *every* target's shim table
whether or not anything calls them. They have been no-op stubs since GC replaced
reference counting in 2018 (pxt-common-packages commit `82106198`), and are byte-identical
between `pxt-common-packages@10.0.2` and `@14.0.2`. They were never removed.

The actual trigger is **`pxt-core` and `pxt-common-packages` resolving to versions from
different eras**. Upstream hit exactly this
([microsoft/pxt-microbit#6134](https://github.com/microsoft/pxt-microbit/issues/6134),
closed 2025-04-07); the maintainer's conclusion was that bumping `pxt-core` requires
bumping `pxt-common-packages` in lockstep. Each `pxt-microbit` release pins a matched
pair, e.g.:

| pxt-microbit | pxt-core | pxt-common-packages |
|---|---|---|
| 5.0.14 | 8.0.8 | 10.0.2 |
| 9.1.1 | 13.0.1 | 14.0.2 |

Common ways the skew gets introduced:

- `npm install -g pxt` + `pxt target microbit` — resolves the target and its runtime
  independently of anything pinned in the repo. This is the usual culprit in stale CI.
- An in-place `npm install` after bumping `pxt-microbit`, leaving stale `pxt-core`
  resolutions in `package-lock.json` from the previous version.
- Overriding `pxt-core` or `pxt-common-packages` directly in `devDependencies`.

## Diagnosis

Check what actually resolved:

```bash
npm ls pxt-core pxt-common-packages pxt-microbit
```

Healthy output has the **top-level `pxt-core` matching what the pinned `pxt-microbit`
declares**:

```
└─┬ pxt-microbit@9.1.1
  ├─┬ pxt-common-packages@14.0.2
  │ └── pxt-core@12.3.1      <- nested; pxt-common-packages' own dep, harmless
  └── pxt-core@13.0.1        <- top level; must match pxt-microbit's pin
```

Confirm the CLI agrees — every `pxt` command prints the pair it is using:

```
Using target microbit with build engine yotta
  target: v9.1.1 .../node_modules/pxt-microbit
  pxt-core: v13.0.1 .../node_modules/pxt-core
```

Cross-check against what the target expects:

```bash
node -e "const p=require('./node_modules/pxt-microbit/package.json');console.log(p.dependencies)"
```

## Fix

1. **Pin only `pxt-microbit`** in `devDependencies`. Never pin `pxt-core` or
   `pxt-common-packages` yourself — let the target choose them.

   ```json
   "devDependencies": {
     "pxt": "^0.5.1",
     "pxt-microbit": "9.1.1"
   }
   ```

2. **Keep the `pxtcli.json` postinstall hook.** With a locally pinned target (rather than
   a global `pxt target` install), the CLI needs it to find the target directory:

   ```json
   "scripts": {
     "postinstall": "node -e \"require('fs').writeFileSync('node_modules/pxtcli.json',JSON.stringify({targetdir:'pxt-microbit'},null,4))\""
   }
   ```

3. **Reinstall clean — this is the part that actually matters.** An in-place
   `npm install` will keep the old resolutions and reproduce the error.

   ```bash
   rm -rf node_modules package-lock.json built pxt_modules
   npm install
   ```

   `built/` and `pxt_modules/` are gitignored build outputs and are safe to delete.

4. **Rebuild and verify.**

   ```bash
   npm ls pxt-core          # top-level version matches pxt-microbit's pin
   npx pxt install          # repopulate pxt_modules/
   npx pxt build            # both mbdal and mbcodal variants
   ```

   Success produces `built/mbdal-binary.hex` (V1), `built/mbcodal-binary.hex` (V2) and the
   combined `built/binary.hex`.

5. **Fix CI too.** If the workflow does `npm install -g pxt && pxt target microbit`, it is
   pulling an unpinned target and will keep failing regardless of what the repo pins.
   Use `npm ci` plus `npx pxt ...` so CI uses the same toolchain as local development.

## If it still fails

Only then look at genuine API compatibility, and check the actual pins rather than
guessing — `node_modules/pxt-microbit/pxtarget.json` records them per variant:

- `compileService.codalTarget.branch` — the `codal-microbit-v2` version used for V2
- base `compileService.githubCorePackage` + `gittag` — the microbit-dal version for V1

Diff those two refs across the versions you moved between and check only the APIs your
C++ actually touches.

## Last resort

Downgrading `pxt-microbit` to a version contemporary with the project's last active
development *will* make the error go away, because it restores a matched pair. Treat this
as a temporary workaround only: it freezes the project on an old CODAL and old runtime,
and the skew will resurface the next time anything moves. Fix the resolution instead.
