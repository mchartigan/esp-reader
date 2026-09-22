# ESP32 E-Reader PCB Project — Context Summary

**Purpose of this file:** Background for continuing this KiCad hardware design project in a new chat session. Paste this in (or attach it) at the start of a new conversation so the assistant has the relevant context without needing the full chat history.

**Last updated:** 2026-09-16, compiled from prior chat decisions plus two KiCad schematic exports: an earlier `ereader.pdf` (5 sheets, boost-converter power architecture) and a superseding `e-reader.pdf` (same 5 sheets, revised power architecture with the boost converter removed). The revised `e-reader.pdf` is the current state of the design; anything from the earlier export that's since changed is marked superseded below rather than deleted, so the history of *why* is preserved.

**Since 2026-09-11:** Connector.kicad_sch's USB-to-UART chip was swapped from CH343G to **FT231XS-U** (supply-chain motivated — sourcing off Chinese-origin silicon), confirmed by a schematic screenshot review. See the revised Connector.kicad_sch section, USB Programming section, and Open Items below for details.

---

## Project Overview

Building a custom single-PCB e-reader in **KiCad**, integrating:
- **ESP32** (WROOM-32E module, per reference design below)
- **Waveshare 7.5" e-Paper V2 display** (GD7965/UC8179 controller, 800×480, B/W)
- **microSD card reader**
- **Buttons** (for UI navigation)
- **LiPo battery** — **Jauch LP675365JU**, with built-in PCM (protection circuit module), charged via an onboard TP4056. Battery-side protection is handled by the cell itself, not a separate board-level protection IC.

Goal: consolidate what's currently a breakout-board-based prototype into one manufacturable PCB.

**Status:** All five planned schematic sheets exist and are populated (root block diagram + MCU, Power, Display, Connector, Storage). Schematic-capture stage — no evidence yet of PCB layout/routing in the reviewed files. The Power sheet was substantially revised in the latest export (boost converter removed); root and Display sheets still have blank date/rev fields in the title block.

---

## Reference Materials Already Reviewed

1. **Waveshare "E-Paper ESP32 Driver Board V3" schematic** (PDF, uploaded earlier)
   - Waveshare's own ESP32 + e-paper driver board — used as a circuit reference, not copied wholesale, though several sections (boost/drive circuit, decoupling value, DIP switch config) turned out to be carried over directly. See "Confirmed matches to the Waveshare reference" below.
   - Confirmed: ESP32-WROOM-32E pinout, e-paper 24-pin FPC pinout, e-paper boost/drive circuit topology, dual 3.3V rail split (EPD_3.3V via RT9193-33 LDO, VDD3V3 via ME6217C33M5G LDO), CH343P USB-to-UART section (reference board uses CH343P; **our design substitutes CH343G — implemented, see USB Programming below**).
   - **No LiPo charging circuit on this reference board** — it's USB/5V powered only. Battery management was designed from scratch (TP4056 charger + Jauch LP675365JU cell — see Power System Notes).

2. **Waveshare "7.5inch e-Paper V2 Specification" datasheet** (PDF, uploaded)
   - Panel: GD7965/UC8179 controller, 800×480 resolution, 163.2×97.92mm active area, 170.2×111.2×1.18mm outline.
   - **FPC connector: 24-pin, 0.5mm pitch.** Contact pad ~0.35±0.05mm wide, FPC tail width 24±0.2mm, insertion depth 6±0.3mm.
   - Full pinout table (Section 1.5-1) confirmed to match the driver board's J1 pinout — pins 1 and 4 are NC per datasheet; **double-check pin-1 orientation against physical FPC silkscreen when building the footprint** — a 1-pin offset on 0.5mm pitch is an easy way to kill the board.
   - Reference power circuit (Section 1.6): L1 = 10µH wire-wound inductor, D1/D2/D3 = MBR0530 Schottky diodes. **Datasheet Section 1.6 text explicitly recommends Si1304BDL or Si1308EDL for Q1** ("otherwise it may affect the normal boost of the circuit") — **however, Waveshare's own driver board V3 schematic uses BSS138LT1G for Q1**, the same part used in this project. Confirmed by directly checking the driver board PDF. So this is a documented discrepancy *within Waveshare's own materials* (datasheet vs. their shipped product), not a deviation this project introduced — treated as low-risk, since it's the same part in a board Waveshare actually manufactures and sells. No action needed.
   - Power budget: image update current 8–12mA, standby 0.215–0.225mA, **deep sleep 2–5µA** — use deep sleep (cmd 0x07 + 0xA5) between refreshes, not standby.
   - **Ghosting/image-sticking risk if panel isn't refreshed within 24 hours** — plan a periodic full-refresh timer in firmware.
   - BS pin: tie LOW for 4-wire SPI (recommended default) — matches schematic (BS pin 8 on J1).

---

## Confirmed matches to the Waveshare reference (not discrepancies)

Several things that looked at first glance like unexplained deviations from datasheet guidance turned out, on checking the actual Waveshare driver board V3 schematic directly, to be intentional carry-overs from that reference design:

- **Q1 = BSS138LT1G** on the e-paper boost/drive circuit — matches the Waveshare driver board exactly (see above).
- **C1 = 47µF bulk decoupling** on the ESP32's VDD_3V3 — matches the Waveshare driver board's value, not the Espressif "ESP Hardware Design Guidelines" corrected figure of 10µF discussed earlier in this project. Since it's a direct match to the reference board (extra margin, not an error), no action needed.
- **SW1, the 2-pole DIP switch on Display.kicad_sch** (with R1 = 2.2R, linking `VDD_CH343`/`RESET2`/`VDD_USB`) — also copied from the reference board, which has a documented config table for this exact switch:

  | | ON: A | OFF: B |
  |---|---|---|
  | USB TO UART | ON | OFF |
  | RESE | 0.47Ω | 3Ω |

  I.e., the switch toggles between two values of a series resistor in the RESE (reset) network depending on whether USB-to-UART passthrough mode is enabled. Not a mystery leftover — has a real documented function inherited from the reference design.

---

## Current Schematic State (superseding notes reflect the latest `e-reader.pdf` export)

Five sheets, matching the planned hierarchy (with "Connectors" realized as "USB to UART" and no separate UserInterface sheet — buttons live on the MCU sheet):

### Root (`e-reader.kicad_sch`)
Block-diagram-only root sheet with sheet symbols for: Microcontroller (`MCU.kicad_sch`), Power (`Power.kicad_sch`), e-Paper (`Display.kicad_sch`), MicroSD Reader (`Storage.kicad_sch`), USB to UART (`Connector.kicad_sch`). No date/rev filled in yet on this sheet.

### MCU.kicad_sch — "ESP32 Module" (dated 2026-09-01) — unchanged in latest export
- **U1 = ESP32-WROOM-32E**, with **C1 = 47µF** bulk decoupling on VDD (confirmed intentional match to Waveshare reference — see above).
- **Four physical buttons:**
  - **SW5 "KEY_RST/USER"** — on EN, pull-up R7 (10k), C15 (100n) debounce. Dual-purpose reset/user button.
  - **SW4 "KEY_FLASH"** — pulls GPIO0 low for bootloader entry, pull-up R6 (10k), C14 (100n).
  - **SW2 "PG_BACK"** and **SW3 "PG_NEXT"** — 10k pull-ups (R4/R5), 100n debounce caps (C12/C13).
- **Test points** on IO12, IO16, IO17, IO21, IO22, IO34, IO35, SENSOR_VP, SENSOR_VN (TP1–TP9).
- **SPI assignment:** two separate ESP32 hardware SPI peripherals:
  - **HSPI** → e-paper: IO13 (MOSI/SDA_EPD), IO14 (CLK/SCL_EPD), IO15 (CS/CS_EPD), plus IO25/IO26/IO27 for BUSY/RST/DC.
  - **VSPI** → microSD: dedicated VSPI CS/MISO/MOSI/CLK.
- GPIO2 and GPIO4 route off-sheet to Power.kicad_sch (activity LED and EPD power-rail control).

### Power.kicad_sch — "Power Management" (dated 2026-09-03) — **substantially revised, boost converter removed**

**Current (latest export):**
- **Battery charging circuit via USB:** U3 = **TP4056-42-ESOP8** linear LiPo charger, standard reference config. R21 = 1.2k sets PROG (~1A charge current) — **⚠️ flagged 2026-09-16, recommend changing to R21 ≈ 2.2k (~450mA)**, see "USB power budget" note below; not yet changed in the schematic as of this writing. R18 = 0.4R in series on the USB input. **D7 (single status LED) via R19 (now 1k, was 10k)** on CHRG — the STDBY-pin status LED (previously D8 + R20 = 10k) has been removed, simplifying to one status LED. Output net **`VBAT`** feeds both the 2-pin battery connector (J3 = `Conn_BATT`) and the rest of the board directly.
  - **⚠️ New — USB power budget (2026-09-16):** J2/Connector.kicad_sch is wired for standard USB2.0 device operation (no BC1.2/PD negotiation), so the board's `bMaxPower` ceiling is 500mA total. R21 = 1.2k alone draws ~800mA–1A charge current, which already exceeds that before adding the FT231XS-U's own ~15–20mA. Recommend R21 ≈ 2.2k (TP4056 formula: I_charge ≈ 1V / R_prog) for ~450mA charge current, leaving headroom under 500mA. **Post-assembly step:** program the FT231XS-U's internal MTP memory via FT_PROG so its USB descriptor's `bMaxPower` field (500mA / 250 in 2mA units) matches actual combined board draw — this is a one-time over-USB configuration step, not a schematic change.
  - Note: TP4056 charge current is not throttled during USB Suspend (no PWREN#-gated load switch implemented) — acceptable for a hobby board per discussion, but a fully USB-compliant "bus powered high power device" would use one of the FT231XS-U's CBUS pins configured as PWREN# for this. Not implemented; not currently planned.
- **Boost converter eliminated.** The previous MT3608-based `VBAT` → `VBAT_5V` boost stage (U4, L2, D10, R22, R23, C25, C26) is **gone entirely**. There is no more boosted-5V rail on this board.
- **USB power is now a pass-through via the TP4056**, not a separate diode-OR'd rail. `VDD_USB` feeds only the TP4056's input; when USB is connected, the TP4056's inherent behavior sources the downstream load from its BAT pin in addition to charging — no separate pass-through diode/switch was added, matching how these ICs normally work.
- **Master power switch (SW6, SPST)** sits directly on `VBAT`, gating it into IC1 (ME6217C33M5G, main 3.3V LDO). Same relative position as before, but now switching raw (unboosted) battery voltage instead of a boosted+OR'd 5V rail.
- **⚠️ Stale net name:** the node downstream of SW6 (feeding both IC1's VIN and the e-paper's dedicated-LDO branch via Q3) is still labeled **`VDD_5V`**, a leftover from the pre-revision boosted topology. It is *not* an orphaned/undriven net — it's the same switched-`VBAT` node used consistently in both places on this sheet — but the name is now misleading, since it's no longer actually 5V. **Recommend renaming to something like `VBAT_SW` before layout**, to avoid future confusion (e.g. someone assuming 5V-rated parts are safe on that net).
- **Dedicated switchable 3.3V rail for the e-paper (`EPD_3V3`):** IC3 = RT9193-33GB, fed from the same switched-`VBAT`/`VDD_5V` node, gated by a P-MOSFET load switch (Q3 = SI2301CDS-T1-E3) driven by an NPN switch (Q2 = S8050) under **GPIO4** control from the ESP32 — unchanged from before, still lets firmware fully power down the e-paper rail between refreshes.
- **Power and activity indicator LEDs:** D4 (always-on power indicator, R8 2.2k off VDD_3V3) and D5 (activity indicator, R9 1k, driven by **GPIO2**) — unchanged.

**⚠️ New consequence of removing the boost converter (flagged, not yet resolved):** both LDOs (IC1 main 3.3V and IC3 EPD 3.3V) now run directly off unboosted battery voltage instead of a clean boosted ~5V. As the LP675365JU discharges toward its cutoff, both LDOs' dropout margin becomes a real constraint rather than a formality — worth checking each LDO's dropout-vs-current curve against expected load before finalizing, especially IC1 under ESP32 WiFi TX current spikes (300–500mA), where battery ESR sag stacks on top of LDO dropout. The EPD rail draws much less current (8–12mA per datasheet) so is lower-risk, but still worth a sanity check against the panel's minimum supply spec near end-of-discharge.

**Superseded (previous export, no longer in the design):** MT3608 boost converter (U4), L2 (6.8µH), D10 (B5819WS), R22 (75k)/R23 (10k) feedback divider, C25/C26 (22µF) — all removed in the revision above. Diode-OR of `VDD_USB` and `VBAT_5V` via D9 (B5819WS) — also removed; USB and battery power no longer meet at an explicit OR node.

### Display.kicad_sch — "e-Paper" (no date/rev filled in) — unchanged in latest export
- **FPC connector: J1 = Hirose `FH12A-24S-0.5SH(55)`**, 24-pin/0.5mm pitch — matches datasheet dimensional requirements.
- **Boost/drive circuit, confirmed matching the Waveshare reference exactly:** L1 = 10µH, D1/D2/D3 = MBR0530, **Q1 = BSS138LT1G** (see "Confirmed matches" above — not a discrepancy).
- **SW1 DIP switch (`SW_DIP_x02`) + R1 (2.2R)** — confirmed matching the reference board's documented USB-TO-UART / RESE config switch (see table above). **Confirmed (2026-09-16):** this switch is what connects `VDD_USB` (post-ferrite-bead VBUS, on Connector.kicad_sch) to `VDD_CH343` (feeding the USB-UART bridge chip's VCC) — they are not the same net directly, they're gated through SW1. This is intentional: with SW1 in the "USB TO UART: OFF" position, the USB-UART bridge chip (now FT231XS-U) is fully unpowered, presumably for battery-only operation where that chip should draw zero current. Not a connectivity bug — matches the documented switch function. (`VDD_CH343` naming is stale post CH343G→FT231XS-U swap — see rename recommendation in Connector.kicad_sch section above; applies to the label on this sheet too.)
- Decoupling: several 1µF/10µF caps (C2–C9) plus C10/C11 (4.7µF) on the boost circuit, per reference topology. (Minor note: some cap values on this sheet — C5/C6/C7/C8 — read as 10µF in the latest export versus 1µF previously; not flagged as a problem, just noted as a difference from the earlier version in case it wasn't intentional.)

### Connector.kicad_sch — "USB to UART" (dated 2026-09-03, **revised 2026-09-16 — CH343G replaced with FT231XS-U**)
- **J2 = USB-C receptacle** (`USB_C_Receptacle_USB2.0_16P`). CC1/CC2 pulled to GND through 5.1k resistors (R10/R11) for USB2.0-only (no PD) device operation. Unchanged.
- **U3 = FT231XS-U, SSOP-20 package** (replaces U2/CH343G-SOP16 — swapped for supply-chain reasons, FTDI is a UK company). Confirmed pinout (per FTDI datasheet): pin1=DTR, pin2=RTS, pin3=VCCIO, pin4=RXD, pin5=RI, pin6=GND, pin7=DSR, pin8=DCD, pin9=CTS, pin10=CBUS2, pin11=USBDP, pin12=USBDM, pin13=3V3OUT, pin14=RESET#, pin15=VCC, pin16=GND, pin17=CBUS1, pin18=CBUS0, pin19=CBUS3, pin20=TXD.
  - No external EEPROM needed — FT231X has internal MTP memory for VID/PID/descriptor config, programmed over USB via FT_PROG (same mechanism as EEPROM on other FTDI parts, no extra hardware).
  - CBUS0–3 (pins 17/18/10/19) left as NC pads for now — configurable I/O, not required for basic operation; optional future use for TX/RX activity LEDs.
  - VCCIO (pin 3) and 3V3OUT (pin 13) tied together (chip's internal regulator supplies its own logic rail); RESET# (pin 14) tied to that same node — no external reset control, matching FTDI's Fig 6.1 bus-powered reference design. GND: pins 6, 16.
  - **USB power/filtering, built per FTDI datasheet Figure 6.1 (Bus Powered Configuration) — new vs. the CH343G section, which had no equivalent filtering:**
    - **FB1** = ferrite bead (Laird MI0805K400R-10, 0805 package) in series between USB-C VBUS and U3 pin 15 (VCC). Footprint: KiCad's generic 0805 hand-solder footprint (confirmed fine — pad geometry is part-agnostic; recommend swapping the *symbol* to `Device:Ferrite_Bead` for schematic readability even though the footprint is correct as-is).
    - **R16/R17 = 27R** in series on USBDM (pin 12)/USBDP (pin 11), between U3 and the USB-C D−/D+ lines.
    - **C16/C17 = 47pF** from D−/D+ to GND, connector-side of R16/R17.
    - **C32 = 10nF** from post-ferrite VCC (labeled `VDD_USB`) to GND.
    - **C30/C31 = 100nF + 4.7µF** bulk decoupling on the `VDD_CH343` rail (see net-naming note below).
    - **C23 = 100nF** on the VCCIO/3V3OUT/RESET# node to GND.
  - **⚠️ Stale net name:** the rail feeding U3's VCC (pin 15) is still labeled `VDD_CH343`, a leftover from the CH343G part — same category of issue as the `VDD_5V` stale name on Power.kicad_sch. **Recommend renaming to something like `VDD_UART_BRIDGE` or `VDD_FTDI`** across Connector.kicad_sch and Display.kicad_sch before layout.
- **Auto-reset/auto-program circuit — unchanged topology, re-pointed to the new chip:** two NPN transistors (Q4, Q5 = S8050) now driven from **FT231XS-U pin 1 (DTR)** and **pin 2 (RTS)** (previously CH343G's DTR/RTS), R24/R25 (10k) bases, C27 (4.7µF)/C28 (1µF) timing caps, drive the ESP32's EN and IO0 nets automatically during flashing (classic ESP32/Espressif dev-board auto-reset pattern — confirmed this is Espressif's own reference circuit, not Waveshare-specific; see esptool docs: https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html). C27 = 4.7µF on the EN-side node is within Espressif's documented 1–10µF recommended range for reset-timing reliability. Bootloader entry works both via this auto-reset circuit and via the manual FLASH/RST buttons on the MCU sheet.
  - **Bring-up note (not a schematic defect):** verify DTR/RTS↔EN/IO0 polarity produces correct esptool auto-reset behavior at first power-up — easy to get swapped when re-sourcing from a different bridge chip, and it only shows up as "auto-flash doesn't work," not as an ERC failure.
- **TXD/RXD:** U3 pin 20 (TXD) → R12 (0Ω, optional rework) → ESP32 RXD0; U3 pin 4 (RXD) → R13 (0Ω, optional rework) → ESP32 TXD0. Unchanged pattern from CH343G version.

### Storage.kicad_sch — "MicroSD Card Reader" (dated 2026-09-09) — unchanged in latest export
- **J4 = `Micro_SD_Card`** (KiCad built-in symbol), standard **SPI mode** wiring: DAT3/CD → SD_CS, CMD → SD_MOSI, CLK → SD_SCLK, DAT0 → SD_MISO.
- Powered from **main VDD_3V3**, not the EPD rail. C29 (100nF) local decoupling — comparatively light for SD inrush/write transients, worth watching during bring-up but not flagged as a hard problem.

---

## KiCad Project Setup Decisions

- **Project-specific symbol/footprint libraries**: set up under `libs/symbols/` and `libs/footprints/*.pretty` inside the project folder, added via Preferences → Manage Symbol/Footprint Libraries → Project Specific Libraries tab, using `${KIPRJ_DIR}` relative paths (not absolute) for portability/git compatibility.
- **Sheet hierarchy** (built out in KiCad, matching the plan with one naming difference — "Connectors" became "USB to UART" and there is no separate UserInterface/Peripherals sheet; buttons live on the MCU sheet):
  ```
  Root (e-reader.kicad_sch) — block-diagram only, sheet symbols + hierarchical labels, no components
  ├── MCU.kicad_sch          — ESP32 module, decoupling, boot/reset circuit, all 4 buttons, test points
  ├── Power.kicad_sch        — Battery charge (TP4056), master switch, dual LDOs (now battery-fed, no boost), LEDs
  ├── Display.kicad_sch      — E-paper FPC connector + boost/drive circuit
  ├── Storage.kicad_sch      — microSD socket
  └── Connector.kicad_sch    — USB-C receptacle, CH343G USB-UART, auto-reset circuit
  ```
  - Hierarchical labels used for nets between exactly 2 sheets (e.g. `SD_CS`).
  - Global labels used for nets touching 3+ sheets — power rails (`VDD_3V3`, `VDD_USB`, `VBAT`, `EPD_3V3`, `GND`) plus cross-sheet control signals like `IO2`/`IO4`. Note the `VDD_5V` label inside Power.kicad_sch (see above) is now a within-sheet-only net despite the name.
- **Net classes**: still not confirmed as configured (not visible from a schematic-only export).
- **Resistor/cap value notation**: plain decimal with unit suffix (`2.2R`, `4.7k`, `100n`) — confirmed consistently applied throughout the schematic.

---

## Component/Part Sourcing Decisions

- **ESP32 module**: Espressif's official KiCad symbols/footprints (GitHub: espressif/kicad-libraries) — implemented as ESP32-WROOM-32E.
- **Battery: Jauch LP675365JU**, LiPo pouch cell with built-in PCM (protection circuit module) — over-discharge/over-current/short-circuit protection lives in the cell itself, not as a separate board-level IC. Resolves the earlier open item about needing a dedicated protection IC.
- **microSD socket**: KiCad's built-in `Micro_SD_Card` symbol/`Connector_Card.pretty` footprint family — still worth verifying the specific footprint variant against the actual part's mechanical drawing before committing to layout.
- **E-paper FPC connector**: Hirose `FH12A-24S-0.5SH(55)`, 24-pin, 0.5mm pitch. Schematic symbol in place; PCB footprint build/verification status not confirmed from schematic review alone (double-check pin-1 orientation against the physical FPC per the datasheet note above).
- **FT231XS-U** (revised 2026-09-16, replaces CH343G): SSOP-20 package, genuine FTDI part sourced via Digikey — chosen specifically to move off Chinese-sourced silicon. Full pinout mapped in schematic (see Connector.kicad_sch section above).
- **General part-sourcing workflow**: KiCad built-in libraries first → manufacturer-published KiCad libraries second → SnapEDA/Ultra Librarian third → hand-build only when necessary. Always cross-check downloaded footprints against actual datasheet dimensions before trusting them.

---

## Power System Notes

- ESP32 WiFi TX current spikes (~300–500mA peaks) require bulk decoupling near the module — **C1 = 47µF** on the MCU sheet, confirmed intentional match to the Waveshare reference board (not the corrected 10µF Espressif figure — see "Confirmed matches" above).
- Two separate 3.3V-ish rails, as planned: `VDD_3V3` for ESP32 logic (and microSD), `EPD_3V3` for the e-paper analog/driver side, gated by its own load switch (Q2/Q3) under GPIO4 control so it can be fully powered down between refreshes.
- **Battery charge circuit: TP4056-42-ESOP8**, ~1A charge rate (R21 = 1.2k PROG), single CHRG status LED, straightforward USB-fed charging into the LP675365JU via a 2-pin battery connector.
- **Battery protection: resolved.** The LP675365JU has built-in PCM; no separate protection IC needed on the board.
- **Power architecture revised: boost converter removed.** Previous topology was LiPo → MT3608 boost → 5V rail, diode-ORed with USB 5V, then down to 3.3V via two LDOs. Current topology: `VBAT` (raw battery, or USB-sourced via the TP4056's pass-through behavior) → master switch (SW6) → directly into both LDOs (main 3.3V and, via the GPIO4-gated load switch, EPD 3.3V). Simpler, but removes the LDOs' input headroom margin — see dropout-margin flag above.
- **Naming cleanup recommended:** rename the post-SW6 net (currently `VDD_5V`) to reflect that it's switched battery voltage, not 5V, before finalizing for layout.

---

## USB Programming — Decided & Implemented

- **Standalone onboard USB programming, fully built out:**
  - **Chip: FT231XS-U, SSOP-20 package** (revised 2026-09-16, replaces CH343G-SOP16 — swapped for supply-chain reasons, FTDI is UK-based). Full pinout remapped in schematic; see Connector.kicad_sch section above for pin table and added USB filtering (ferrite bead + 27R/47pF per FTDI's bus-powered reference design, which the CH343G version didn't have).
  - No external EEPROM required — FT231X uses internal MTP memory for descriptor config, programmable over USB via FT_PROG.
  - **USB-C receptacle**, with correct CC1/CC2 5.1k pull-downs for USB2.0-only (no PD) device operation. Unchanged.
  - **Auto-reset/auto-program circuit** (two-NPN-transistor DTR/RTS driver, standard Espressif dev-board pattern — confirmed via esptool's official docs, not just a Waveshare convention) — unchanged topology, re-pointed to FT231XS-U's DTR (pin1)/RTS (pin2). Bootloader entry works automatically via `esptool`/IDE flashing tools, in addition to the manual FLASH/RST buttons on the MCU sheet.
  - **USB power now also feeds the battery charger directly** (see Power System Notes) rather than joining a separate boosted system rail — i.e., USB's role is charging the battery and (via the TP4056's pass-through behavior) supplementing system power, not independently boosting to 5V.
  - **Open action item:** R21 (TP4056 PROG resistor) needs changing from 1.2k to ~2.2k to bring charge current under the 500mA USB2.0 budget once the FT231XS-U's own draw is included — see Power System Notes.
  - **Open action item:** post-assembly, program FT231XS-U's `bMaxPower` descriptor via FT_PROG to match actual board current draw.

---

## GPIO / Signal Summary (from schematic)

| Signal | Function |
|---|---|
| GPIO0 | Manual FLASH button (SW4) + auto-reset transistor (Q5) — bootloader entry |
| EN | Manual RST/USER button (SW5) + auto-reset transistor (Q4) — chip reset |
| GPIO2 | Drives D5 activity indicator LED |
| GPIO4 | Controls Q2/Q3 load switch — gates EPD_3V3 rail on/off |
| IO13/14/15 (HSPI) | E-paper SDA/SCL/CS (MOSI/CLK/CS) |
| IO25/26/27 | E-paper BUSY/RST/DC |
| VSPI (dedicated) | microSD CS/MISO/MOSI/CLK |
| IO12,16,17,21,22,34,35, SENSOR_VP/VN | Test points only (TP1–TP9) |

---

## Open Items / Not Yet Decided

1. ~~LiPo battery protection~~ — **Resolved**: Jauch LP675365JU has built-in PCM; no separate protection IC needed.
2. ~~microSD ↔ ESP32 wiring~~ — **Resolved**: dedicated hardware VSPI bus for microSD, separate from the e-paper's HSPI bus.
3. ~~CH343G symbol/footprint/pin remap~~ — **Resolved**: implemented with SOP16 pinout and an added auto-reset circuit.
4. **Net class definitions** — still not confirmed as configured in KiCad (Setup → Net Classes); not visible from a schematic-only export.
5. **DRC rules / fab selection** — still open; fab not yet chosen.
6. **Mechanical/enclosure coordination** — board outline, mounting holes, button/display cutouts not yet started.
7. ~~Sheet-by-sheet schematic capture~~ — **Done**: all five sheets built out and populated. PCB layout/routing not yet started.
8. **FPC footprint** — connector part chosen (Hirose FH12A-24S-0.5SH(55)) and used as a schematic symbol; actual PCB footprint build/verification in the Footprint Editor not confirmed.
9. ~~MOSFET substitution risk~~ — **Resolved, not actually a risk**: Q1 = BSS138LT1G matches the Waveshare reference board exactly.
10. ~~SW1/R1 jumper purpose~~ — **Resolved**: matches the Waveshare reference's documented USB-TO-UART/RESE config switch.
11. ~~Decoupling value discrepancy~~ — **Resolved, not actually a discrepancy**: 47µF matches the Waveshare reference intentionally.
12. **NEW — `VDD_5V` net naming** in Power.kicad_sch is stale/misleading post-boost-removal; recommend renaming before layout (cosmetic, not functional).
13. **NEW — LDO dropout margin near end-of-discharge**, now that both LDOs run directly off unboosted battery voltage. Needs a check against the LP675365JU's cutoff voltage and each LDO's dropout-vs-current curve, particularly IC1 (main 3.3V) under ESP32 TX current spikes.
14. **NEW — Display.kicad_sch decoupling cap values** (C5–C8) appear as 10µF in the latest export vs. 1µF previously — worth a quick sanity check that this was intentional and not a stray edit.
15. ~~CH343G supply-chain concern~~ — **Resolved 2026-09-16**: replaced with FT231XS-U (genuine FTDI, UK-based), full pinout/filtering re-implemented on Connector.kicad_sch per FTDI's bus-powered reference design.
16. **NEW — `VDD_CH343` net naming** (Connector.kicad_sch and Display.kicad_sch) is stale post CH343G→FT231XS-U swap; recommend renaming to `VDD_UART_BRIDGE` or similar before layout (cosmetic, not functional — same category as item 12).
17. **NEW — R21 charge-current resistor** should change from 1.2k to ~2.2k so TP4056 charge current + FT231XS-U draw stays under the 500mA USB2.0 device power budget. Not yet updated in the schematic as of 2026-09-16.
18. **NEW — FT231XS-U `bMaxPower` USB descriptor** needs programming via FT_PROG after assembly, to match actual combined board current draw. Post-assembly step, not a schematic item.
19. **NEW — CBUS0–3 pins** on FT231XS-U left as NC for now; optional future use (e.g., TX/RX activity LEDs) not yet decided.
20. **NEW — Auto-reset DTR/RTS↔EN/IO0 polarity** should be verified at first bring-up now that the bridge chip changed — functionally should be correct (same circuit topology, doc-confirmed against Espressif's reference), but worth confirming on hardware since a swap is an easy place to introduce a polarity mixup that ERC wouldn't catch.

---

## Useful References Collected So Far

- Waveshare "E-Paper ESP32 Driver Board V3" schematic PDF (user-provided)
- Waveshare "7.5inch e-Paper V2 Specification" datasheet PDF (user-provided)
- Espressif ESP Hardware Design Guidelines: https://docs.espressif.com/projects/esp-hardware-design-guidelines/
- Espressif official KiCad libraries: github.com/espressif/kicad-libraries
- Jauch LP675365JU battery datasheet (referenced in chat; not yet uploaded to this project — worth attaching if further power-budget/dropout analysis is needed)
- FTDI FT231X datasheet (referenced in chat, esp. Section 9 pinout tables and Figure 6.1 bus-powered reference design): https://ftdichip.com/wp-content/uploads/2025/06/DS_FT231X.pdf
- esptool official docs, ESP32 auto-reset circuit + truth table (confirms the two-transistor DTR/RTS circuit is Espressif's own reference design): https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html
- Espressif flashing-troubleshooting docs, points to the ESP32 DevKitC schematic as the canonical auto-reset reference: https://docs.espressif.com/projects/esp-idf/en/latest/get-started/flashing-troubleshooting.html
- KiCad schematic exports: `ereader.pdf` (superseded — boost-converter power architecture) and `e-reader.pdf` (current — boost removed, battery-direct power architecture), KiCad E.D.A. 10.0.5, 6-page exports (root + 5 sheets)
