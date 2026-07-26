---
name: pxt-build-troubleshooting
description: Use when pxt test or pxt build fails with C++ compiler errors like "Non-thumb addr" in an older MakeCode project.
---

# PXT Build Troubleshooting

When running `pxt test` or `pxt build` on an older MakeCode extension project, you might encounter an internal compiler error during the C++ build phase, such as:

```
INTERNAL ERROR: Error: Non-thumb addr for pxt::decr
```

## Cause

This happens when the globally installed `pxt` CLI (or `npx pxt`) attempts to use the latest version of the `pxt-microbit` target and its corresponding `pxt-core`. The modern toolchain may be incompatible with the custom C++ GATT service code, CODAL integrations, or `yotta` dependencies of a project that was last updated several years ago.

## Solution

The solution is to pin the `pxt-microbit` target to a version that was contemporary with the project's last active development, and configure the local toolchain to use it.

1. **Identify the project's last active date:** 
   Check the git history or the last modified date of the project (e.g., late 2022).

2. **Downgrade `pxt-microbit`:** 
   Find and install a version of `pxt-microbit` that was published around that time (e.g., `5.0.14`).
   ```bash
   npm install pxt-microbit@<compatible-version> --save-dev
   ```

3. **Auto-generate `pxtcli.json`:** 
   When you pin the version locally, the `pxt` CLI needs a `pxtcli.json` file in `node_modules/` to correctly locate the target directory. Add a `postinstall` script to your `package.json` to handle this automatically:
   ```json
   "scripts": {
     "postinstall": "node -e \"require('fs').writeFileSync('node_modules/pxtcli.json',JSON.stringify({targetdir:'pxt-microbit'},null,4))\"",
     ...
   }
   ```

4. **Clean and Test:**
   After updating `package.json`, clean the environment and test again:
   ```bash
   rm -rf built node_modules
   npm install
   npm run test:pxt
   ```

By explicitly pinning the target version, the PXT cloud compiler will correctly utilize the older, compatible toolchain (e.g., `pxt-core@8.x` instead of `13.x`), allowing the C++ compilation and linking process to succeed without "Non-thumb addr" errors.
