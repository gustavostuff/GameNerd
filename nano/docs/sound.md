# Sound (Nano)

**Status: design.** Ahead of firmware and Nano Studio audio UI.

## Channels

Nano uses **two independent** audio channels (not the full Retr01 5-lane BGM + 3 SFX map):

| Channel | Role | Wave (planning) |
|---------|------|-----------------|
| **Music** | BGM / pulse lead | Hardware PWM square (Pulse-like) |
| **SFX** | One-shots, hits, UI | Second PWM (pulse or noise-style by firmware) |

Studio authoring will expose these as two lanes (music + SFX), not five overlapping BGM tracks.

## Hardware impact

**Small.** The ATmega1284P already has several timer PWM outputs (`OC0` / `OC1` / `OC2` pins).

Typical board change vs a single mono PWM:

1. Route a **second PWM pin** for SFX.
2. Mix Music + SFX with **two resistors** (and the existing DC block) into the same audio jack.
3. Optional: separate volume trims later. Not required for v1.

No extra codec, no second MCU, no cart audio IC. Pin budget grows by **one GPIO**. Firmware updates two compare registers in VBlank (or on SFX trigger) instead of one.

### Alternatives (not preferred)

| Approach | Pros | Cons |
|----------|------|------|
| Software-mix both into **one** PWM | Zero extra pins | Music and SFX fight for duty. Harder "independent" feel |
| Two jacks | True split | Larger PCB. Overkill for Nano |

Preferred: **two PWM pins, resistor mix, one audio out.**

## Firmware sketch

- Music: pitch/duty from cart song data or Studio export, ticked in VBlank.
- SFX: fire-and-forget from game / pad hooks. May preempt or layer under music via the analog mix.
- Both stay off the active video kernel path (same rule as SPI: no heavy work mid-scanline).

## Studio direction

Nano Studio Audio UI shrinks to:

- **Music plane:** single Pulse lane (PWM music)
- **SFX plane:** single channel editor / one-shot list (placeholder until authored)

Host preview can keep using the softsynth, mapped onto these two roles, until cart-protocol audio exists.

## Relation to full Retr01

Full Retr01 targets a richer APU on a separate MCU. Nano keeps audio on the **same 1284P** as video and pads, with a deliberately tiny channel count.
