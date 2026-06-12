# ADS1220 Library Modernization Plan

Bring `Protocentral_ADS1220` up to the v2 ProtoCentral house style, matching
`ProtoCentral_ads1262` (the freshly-rewritten sibling TI delta-sigma ADC), while
keeping the existing MIT license, header format, and `license_mark.svg` assets.

## Goals
1. Modern enum/struct-based API with Doxygen docs, blocking + async reads.
2. SPDX + block-comment header copied from the ads1262 library (year `2018-2026`).
3. Per-instance `SPIClass*` + ESP32 `begin(sck,miso,mosi)` overloads.
4. Expand from 2 → 8 examples covering the full ADS1220 feature set.
5. Add `keywords.txt`, `compile_examples.sh`, multi-board CI, refreshed README.
6. **Clean break** — remove the old v1.2.1 API entirely (major version bump 2.0.0).

---

## 1. Core library rewrite (`src/`)

Keep filenames `Protocentral_ADS1220.h/.cpp` (avoid breaking `#include`), but
modernize internals. New header layout mirrors `ads1262.h`:

### Header / license
- Replace `//` banner with the SPDX block-comment header from `ads1262.h`.
- `Author: Ashwin Whitchurch, Protocentral Electronics`, `support@protocentral.com`,
  `Copyright (c) 2018-2026 ProtoCentral Electronics`, MIT text verbatim.
- `#define ADS1220_LIBRARY_VERSION "2.0.0"`.

### Register / command map
- Keep `#define`s for SPI commands (RESET/START/SYNC/WREG/RREG/POWERDOWN) and the
  4 config register addresses, namespaced `ADS1220_CMD_*` / `ADS1220_REG_*`.

### Typed enums (replace raw `#define` config values)
- `ADS1220_DataRate`   — 20/45/90/175/330/600/1000 SPS (normal); note turbo doubles.
- `ADS1220_OpMode`     — NORMAL / DUTY_CYCLE / TURBO.
- `ADS1220_Gain`       — 1,2,4,8,16,32,64,128.
- `ADS1220_Mux`        — full 16-entry MUX table (differential, AINx-AVSS single-ended,
  VREF/4 and AVDD/4 diagnostics, shorted-for-offset).
- `ADS1220_Reference`  — INTERNAL_2_048V / EXTERNAL_REFP0_REFN0 / EXTERNAL_AIN0_AIN3 / ANALOG_SUPPLY.
- `ADS1220_FIRFilter`  — OFF / 50_60Hz / 50Hz / 60Hz.
- `ADS1220_IDACCurrent`— OFF/10/50/100/250/500/1000/1500 µA.
- `ADS1220_IDACRouting`— disable/AIN0..AIN3/REFP0/REFN0 (for IDAC1 & IDAC2).
- `struct ADS1220_Config { dataRate; opMode; gain; mux; reference; bool pgaBypass;
  bool continuousMode; ... }`.

### Class `Protocentral_ADS1220` — new clean API
Initialization:
- `Protocentral_ADS1220();` (default pins)
- `Protocentral_ADS1220(uint8_t csPin, uint8_t drdyPin);`
- `Protocentral_ADS1220(uint8_t csPin, uint8_t drdyPin, SPIClass* spi);`
- `bool begin(bool startSPI = true);`
- `bool begin(const ADS1220_Config& cfg, bool startSPI = true);`
- `bool begin(uint8_t sck, uint8_t miso, uint8_t mosi);` (ESP32)
- `void end();`

Configuration (camelCase, chainable-by-name):
- `setDataRate`, `setOperatingMode`, `setGain`, `enablePGA(bool)`,
  `setInputMux(ADS1220_Mux)`, `setInputMuxDiff(pos,neg)`,
  `setReference`, `setFIRFilter`, `setConversionMode(continuous?)`,
  `enableTemperatureSensor(bool)`, `setIDACCurrent`, `setIDAC1Route`, `setIDAC2Route`,
  `enableLowSideSwitch(bool)`, `setDRDYMode(...)`.

Acquisition:
- `void startConversion();` / `void stopConversion();` / `bool isDataReady() const;`
- `int32_t readRaw();`           (blocking, single-shot or continuous aware)
- `float   readVoltage();`        (auto V conversion from ref + gain)
- `bool    readRawAsync(int32_t&);`
- `bool    readVoltageAsync(float&);`
- `float   readTemperature();`    (enables temp sensor, reads, restores mux)
- `int32_t readSingleEnded(uint8_t channel);` (convenience, single-shot)

Utility:
- `void reset();`, `bool testConnection();`, `void powerDown();`
- `void writeRegister(addr,val);` / `uint8_t readRegister(addr);`
- `void getConfigRegisters(uint8_t out[4]);` / `void printRegisters(Stream& = Serial);`
- `float countsToVoltage(int32_t) const;` / `int32_t voltageToCounts(float) const;`
- `static const char* getLibraryVersion();`

Internals: cached `_configReg[4]`, `_vref`, `_gain`, `_pgaBypass`, `_csPin`,
`_drdyPin`, `_spi`, `SPISettings` built per-instance, MSBFIRST/SPI_MODE1.

### Backward-compatibility
**Clean break (user-approved).** The old v1.2.1 method names
(`select_mux_channels`, `set_pga_gain`, `Read_SingleShot_SingleEnded_WaitForData`,
`Read_Data_Samples`, `internal_reference`, …) and raw `#define` config constants
(`DR_330SPS`, `PGA_GAIN_1`, `MUX_SE_CH0`, …) are removed entirely. The 2.0.0 major
bump signals the breaking change; existing sketches must move to the enum API.
README documents the migration.

---

## 2. Examples (expand 2 → 8)

Numbered `NN-Name/NN-Name.ino`, each with the SPDX header, a wiring table
(Arduino + ESP32 columns), and feature list — matching ads1262 examples.

| # | Example | Showcases |
|---|---------|-----------|
| 01 | `01-Basic-SingleEnded-Read` | `begin()`, single-ended scan of AIN0–AIN3, `readVoltage()`, mV output |
| 02 | `02-Differential-Continuous-Stream` | continuous mode, DRDY interrupt, `readRawAsync`, high-rate streaming |
| 03 | `03-Internal-Temperature-Sensor` | `readTemperature()`, internal temp-sensor mode |
| 04 | `04-High-Resolution-PGA` | PGA gain 1→128 sweep, low-level µV measurement, noise demo |
| 05 | `05-RTD-PT100-3Wire` | dual IDAC current sources + ratiometric external ref — classic ADS1220 PT100 RTD app, °C output |
| 06 | `06-Loadcell-Wheatstone-Bridge` | ratiometric bridge measurement, external ref = excitation, low-side switch, gain 128 |
| 07 | `07-Register-Dump-Diagnostics` | `printRegisters()`, `testConnection()`, `readDeviceID`-style self-check, config struct usage |
| 08 | `08-ESP32-MultiSPI` | `begin(sck,miso,mosi)`, custom `SPIClass*`, `ARDUINO_ARCH_ESP32` guards |

(05/06 are the headline additions — they exercise the IDAC current sources,
external/ratiometric reference, and low-side switch that the current library
exposes but never demonstrates.)

Add `examples/compile_examples.sh` (arduino-cli loop, ✅/❌ summary) like ads1262.

---

## 3. Project metadata & tooling

- **`library.properties`** → bump to `2.0.0`; author/maintainer
  `Ashwin Whitchurch, Protocentral Electronics <support@protocentral.com>`;
  richer `paragraph` (enum API, blocking/async, RTD/bridge/temp examples);
  add `depends=SPI`, `includes=Protocentral_ADS1220.h`.
- **`keywords.txt`** (new) — KEYWORD1 for class + enum types, KEYWORD2 for every
  public method, LITERAL1 for all enum constants. Modeled on ads1262 `keywords.txt`.
- **`.github/workflows/compile-examples.yml`** → replace with the ads1262 multi-board
  matrix (AVR/SAMD/ESP32/ESP8266/RP2040/STM32/nRF52) so all 8 examples are CI-built.
- **`README.md`** → keep hero image + buy link + license section; rewrite Features/
  API/Examples sections to document the new API, add an examples table and a
  quick-start snippet. Keep `LICENSE.md` and `license_mark.svg` unchanged.
- Leave `docs/`, `hardware/`, `python/` as-is (out of scope; `python/continuos.py`
  is Py2 — note it but don't touch unless requested).

---

## 4. Verification
- Compile all 8 examples locally via `examples/compile_examples.sh` against an
  installed board (e.g. Arduino Uno + ESP32) using `arduino-cli`.
- Confirm the deprecated shim lets the two original sketches compile unchanged.
- Push relies on the CI matrix to validate across all target architectures.

## Out of scope (unless you ask)
- Rewriting the Raspberry Pi `python/` driver.
- Regenerating hardware/PDF docs.
- Publishing/tagging a release.
