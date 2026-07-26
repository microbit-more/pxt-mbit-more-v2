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

## Usage & Build

1. Open [MakeCode for micro:bit](https://makecode.microbit.org/)
2. Search and import `https://github.com/microbit-more/pxt-mbit-more-v2` via the **Extensions** menu
3. Download (flash) the program to your micro:bit
