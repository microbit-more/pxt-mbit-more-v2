#!/usr/bin/env node
/**
 * Propagate the version in package.json to every other place that carries it.
 *
 * package.json is the single source of truth. `npm version <level>` runs this
 * through the `version` lifecycle hook, so the bump lands in one commit.
 * `npm test` runs it with --check so a hand-edit that drifts fails CI.
 *
 *   node scripts/sync-version.js            rewrite the targets
 *   node scripts/sync-version.js --check    report drift and exit 1
 */

const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..');
const check = process.argv.includes('--check');

const version = JSON.parse(
  fs.readFileSync(path.join(root, 'package.json'), 'utf8')
).version;

if (!/^\d+\.\d+\.\d+/.test(version)) {
  console.error(`sync-version: unexpected version in package.json: "${version}"`);
  process.exit(1);
}

// Each target matches exactly once. The capture group is the version itself, so
// the replacement can rebuild the line without knowing its shape.
const targets = [
  {
    file: 'pxt.json',
    pattern: /("version"\s*:\s*")([^"]*)(")/,
    describe: 'PXT package version'
  },
  {
    file: 'MbitMoreCommon.h',
    pattern: /(#define\s+MBIT_MORE_VERSION_STRING\s+")([^"]*)(")/,
    describe: 'version shown on the LED matrix at startup'
  },
  {
    file: 'AGENTS.md',
    pattern: /(- \*\*Version\*\*:\s*)([^\s]+)(\s*$)/m,
    describe: 'version stated in the docs'
  }
];

const drifted = [];
const written = [];

for (const target of targets) {
  const file = path.join(root, target.file);
  const source = fs.readFileSync(file, 'utf8');

  // Guard against a silent no-op if a file is restructured later.
  const hits = source.match(new RegExp(target.pattern.source, target.pattern.flags.replace('g', '') + 'g'));
  if (!hits || hits.length !== 1) {
    console.error(
      `sync-version: expected exactly 1 version match in ${target.file}, found ${hits ? hits.length : 0}.\n` +
      `  The pattern for the ${target.describe} needs updating in scripts/sync-version.js.`
    );
    process.exit(1);
  }

  const current = source.match(target.pattern)[2];
  if (current === version) continue;

  if (check) {
    drifted.push(`  ${target.file}: ${current}  (expected ${version} -- ${target.describe})`);
    continue;
  }

  fs.writeFileSync(file, source.replace(target.pattern, `$1${version}$3`));
  written.push(`  ${target.file}: ${current} -> ${version}`);
}

if (check) {
  if (drifted.length) {
    console.error(`sync-version: version drift against package.json (${version}):`);
    console.error(drifted.join('\n'));
    console.error('\nRun `npm run sync-version` to fix.');
    process.exit(1);
  }
  console.log(`sync-version: all version references match package.json (${version})`);
} else if (written.length) {
  console.log(`sync-version: synced to ${version}`);
  console.log(written.join('\n'));
}
