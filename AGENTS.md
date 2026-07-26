# AGENTS.md - pxt-mbit-more-v2

This document is a guide for AI agents (Antigravity, GitHub Copilot, etc.) to understand the `pxt-mbit-more-v2` project.

## Project Overview

**pxt-mbit-more-v2** is the micro:bit firmware and extension repository for Microbit More v2.
It includes MakeCode (PXT) TypeScript block definitions and C++ (CODAL / microbit-dal) custom BLE GATT service implementations.

- **Name**: pxt-mbit-more-v2
- **Version**: 0.2.5
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
# Install dependencies
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

## CODAL Resources

For C++ development targeting the micro:bit v2, the following resources are useful:
- [CODAL Documentation](https://lancaster-university.github.io/codal-documentation/index.html)
- [codal-microbit-v2 Repository](https://github.com/lancaster-university/codal-microbit-v2)
- [microbit-v2-samples Repository](https://github.com/lancaster-university/microbit-v2-samples)
