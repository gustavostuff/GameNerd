# Two-layer PCB guide (KiCad)

A practical walkthrough for finishing a board like the Nano cart: parts placed, ratsnest visible, copper not routed yet. Aimed at **2-layer** boards in KiCad (PCB Editor).

## What a 2-layer board is

| Layer | Usual KiCad name | Typical use |
|-------|------------------|-------------|
| Top copper | `F.Cu` (Front) | Most signal traces, component pads (THT pads go through both) |
| Bottom copper | `B.Cu` (Back) | Ground pour, and/or more signal traces |
| Board outline | `Edge.Cuts` | Shape of the PCB |
| Silkscreen | `F.SilkS` / `B.SilkS` | Refdes, labels (optional on bottom) |

Through-hole pads (DIPs, headers, cart fingers) have copper on **both** sides by default. Surface-mount pads live on one side only.

You only have **two** copper sheets. That is enough for many small boards if you plan pours and a few vias carefully.

## Bottom copper: ground only, or traces too?

**Both are valid.** You are not wrong.

Common patterns on 2-layer boards:

1. **Bottom mostly ground pour**  
   Large copper fill on `B.Cu` tied to `GND`. Top carries most signals. Short jumps to the bottom when the top is blocked. Best for noise, return paths, and hand soldering heat sinking.

2. **Bottom used as a second signal layer**  
   When top has no clear path from A to B, drop a **via**, run on `B.Cu`, via back up. Very normal. You may still want a ground pour in the leftover space on bottom (or top).

3. **Split roles**  
   Example: bottom = GND pour + a few power or stubborn nets. Top = SPI/I2C/signals.

What to avoid:

- Long parallel runs of fast clocks on both sides of a thin board without thinking about return path (less critical on a tiny slow SPI cart, more critical on high-speed digital).
- Leaving large floating copper islands (connect pours to a net, usually `GND`, or delete unused copper).

**Rule of thumb for Nano-style carts and bring-up mobos:** pour **GND on the bottom**, route signals on top, and use the bottom for **short escape traces** when top is crowded. That matches your mental model and is a solid default.

## Before you draw copper

1. **Open the PCB** that already has footprints + netlist (ratsnest lines).
2. Confirm **Board Setup -> Layers**: at least `F.Cu` and `B.Cu` enabled (default for 2-layer).
3. **Board Setup -> Design Rules -> Constraints**: set sensible minima for your fab (examples for cheap 2-layer):
   - Clearance ~ 0.2 mm
   - Track width ~ 0.25-0.3 mm for signals. Wider for power (0.5-1.0 mm)
   - Via diameter / drill per fab (e.g. 0.6 / 0.3 mm)
4. **Net classes** (optional): give `+5V` and `GND` a wider track class.
5. Lock **Edge.Cuts** and keep the cart finger geometry alone until routing is done.

Ratsnest is only a reminder of connectivity. Copper traces (and pours) are what the fab builds.

## Routing traces in KiCad (PCB Editor)

### Basic signal trace

1. Press **X** (or toolbar **Route Tracks**).
2. Click the starting pad (or an existing trace end).
3. Move toward the target. Click to place corners.
4. Click the destination pad to finish. Ratsnest for that connection disappears when the net is fully connected.
5. **End** / Esc exits route mode.

Tips:

- Stay on **one layer** until you need to cross something.
- Prefer short, direct runs. Avoid sharp acute angles if you can (45 deg bends are fine).
- For THT DIPs, leave room for the iron. Do not pack traces under where you need access if you hand-solder.

### Changing layer mid-route (via)

When top has no clear path:

1. While routing, press **V** to place a **via** (or switch layer with the layer hotkeys / click the layer tab).
2. Continue on `B.Cu`.
3. Via back to `F.Cu` near the destination if the pad is easier to hit from the top.

Vias are plated holes that connect `F.Cu` to `B.Cu`. Use them freely on 2-layer boards. Just do not sprinkle dozens under a tiny DIP if a short bottom hop will do.

Hotkeys worth remembering:

| Key | Action |
|-----|--------|
| **X** | Route track |
| **V** | Place via (while routing) |
| **PgUp / PgDn** | Front / back layer (or use layer selector) |
| **E** | Edit (click a track/via) |
| **Delete** | Remove selected copper |
| **B** | Fill copper zones (after zones exist) |
| **Ctrl+B** | Rebuild all zones |

Exact keys can vary slightly by KiCad version. Check **Preferences -> Hotkeys** if one does not match.

## Ground (and power) pours

A **zone** is a filled copper area attached to a net.

### Add a bottom ground pour

1. Switch active layer to **B.Cu**.
2. **Add -> Copper Zone** (or toolbar zone tool).
3. Outline the board inside `Edge.Cuts` (click corners, double-click / finish to close).
4. In the dialog: set **Net** to `GND`, layer `B.Cu`, clearance and minimum width from your rules.
5. Press **B** to fill.

You should see solid (or hatched) copper on the bottom connected to every GND pad it can reach. Gaps appear around tracks and pads per clearance.

Optional:

- Light **F.Cu** GND pour in empty top areas (helps, but keep soldering pads clear).
- Separate **+5V** zone only if the board is simple and you understand splits. Many small boards just use wide traces for +5V instead.

### Keep pours healthy

- After moving tracks, press **B** again so fills update.
- Use **Inspect -> Continuity** / DRC before fab.
- Cart edge: keep finger pads and soldermask openings as the footprint defines them. Do not flood over gold-finger geometry carelessly.

## Suggested order for a board like the cart image

Your layout already has U25 (25LC1024), U50 (24C64), bypass caps, 0R bridges, and J16 fingers. A calm route order:

1. **Power first**  
   `+5V` from J16 to the 0R bridges / VCC pins with a **wider** track. `GND` stubs short to pads that will sit in the pour.
2. **SPI cluster** (J16 -> U25)  
   SS#, SCK, MOSI, MISO as a neat group on `F.Cu` if possible.
3. **I2C** (J16 -> U50)  
   SDA, SCL. Leave space for pull-ups if they live on the mobo (they do on Nano).
4. **Local bypass**  
   CD1/CD2 should hug U25/U50 VCC-GND with very short leads (already placed near the chips is good).
5. **Bottom pour GND**, then **B** to fill.
6. **Unresolved ratsnest**: use vias + short `B.Cu` hops, then refill zones.
7. **DRC** (Design Rules Checker). Fix clearance, unconnected items, courtyard overlaps.
8. Silkscreen tidy-up, then plot Gerbers / fab outputs.

Mounting holes: if the center hole is non-plated mechanical only, keep copper clearance. If it should tie to GND, use a plated hole or a pad on the GND net.

## Crossings and "no path on top"

| Situation | Typical fix |
|-----------|-------------|
| Two signals must cross | One stays on `F.Cu`, the other vias to `B.Cu` and back |
| Trace blocked by a pad row | Arc around, or via under/ beside and run on bottom |
| Power needs to cross a signal bundle | Power on bottom for a short span, or rearrange parts |
| Everything fights on a tiny board | Pour GND on bottom. Accept more vias. Widen outline slightly |

You do **not** need a 4-layer board for this cart class of design.

## Checks before you order

1. **Ratsnest empty** (or only intentional no-connects).
2. **DRC clean** for your rule set.
3. **3D viewer**: parts fit, DIP orientation (pin 1), cart edge direction.
4. Confirm fab accepts your min track/via/clearance.
5. Export Gerbers + drill per fab instructions (File -> Fabrication Outputs).

## Nano cart notes (context for the screenshot)

- Edge nets are few: GND, +5V, SPI, I2C. That is an easy 2-layer route.
- Fully THT DIPs: route on top where possible. Bottom pour helps heat and GND.
- `RD*` 0 ohm parts are for Quilter-style local +5V islands on denser boards. On a tiny cart they still just bridge as short links. Treat them as wire jumpers in copper if that matches your netlist intent.
- Do not confuse **edge pad count (8+8)** with **IC pin count**. Fingers are the connector. Chips sit inland.

## Short glossary

| Term | Meaning |
|------|---------|
| Ratsnest | Straight-line "air wires" showing unrouted connections |
| Track / trace | Copper path you draw |
| Via | Plated hole joining layers |
| Zone / pour | Filled copper region on a net |
| DRC | Automatic rule check |
| THT | Through-hole (pads on both sides) |

## Further reading in this repo

- Nano cart / mobo hardware: [`nano/docs/hardware.md`](../nano/docs/hardware.md)
- Cart edge recipe: [`docs/cart.md`](cart.md)
- Full Retr01 passives / fab-ish notes: [`docs/passive_rf_etc.md`](passive_rf_etc.md)

KiCad menus move slightly between versions. If a command name differs, search the PCB Editor command palette / Preferences hotkeys for "Route", "Via", and "Copper Zone".
