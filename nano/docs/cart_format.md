# Cart / ROM format (Nano)

**Status: design + Studio export.** Host tools write this image today. Console firmware loaders are still ahead.

Magic file: **`.r01nano`** (packed). Cart image: **`_flash.bin`** padded to **128 KB** (`0xFF`) for **25LC1024** (1 Mbit SPI EEPROM).

Project authoring file: **`.r01proj`** JSON with `"platform": "nano"` and `"version": 1`.

## Relation to full Retr01

| Idea from full `.retr01` | Nano |
|--------------------------|------|
| 6-byte magic + ver + flags header | Yes (`r01nan`, ver 1) |
| Pointer table of `{off u24, len u24}` | Yes (4 slots, not 6) |
| World directory 8 slots x 8 B | Yes |
| Per-world blob with CHR + screen dir + payloads | Yes |
| Entity type + instance tables in world | Yes (simplified) |
| 32 KB 6502 PRG | **No** |
| Global BG/SPR palette planes | **No** (8 fixed FG levels on board) |
| SPR CHR / BG0 / other-screens blob | **Not in v1 export** |
| 512 KB flash pad | **128 KB** |

## Packed layout

```text
0x0000  Header 16 B
        magic[6]="r01nan", format_ver=1, flags=0, reserved[8]

0x0010  Pointer table 24 B (4 slots)
        [0] world_directory
        [1] music_blob (may be empty)
        [2] reserved (len 0)
        [3] reserved (len 0)

0x0028  World directory 64 B
        8 x { present u8, pad u8, off u24, len u24 }
        Studio v1 exports world 0 only

        World blob(s) follow (see below)

        Optional music blob (slot 1)

0x20000 End of flash image (pad with 0xFF)
```

Offsets in the pointer table and world directory are **absolute** file offsets. Offsets inside a world blob are **relative to that world base**.

## World blob

```text
32 B header
  [0]     spawn_cell  col|(row<<4)
  [2]     default_bg_bank
  [4]     default_fg (0-7)
  [5]     present_screen_count (max 16)
  [7]     flags
  [8-10]  off_chr
  [11-13] off_screen_dir
  [17]    entity_type_count
  [18]    entity_instance_count
  [19-21] off_entity_types
  [22-24] off_entity_instances
  [25]    player_entity (0xFF = none)
  [26-29] player hitbox x,y,w,h (tile-sized 0,0,8,8)

CHR
  4 banks x 2048 B = 8192 B
  Each bank: 256 tiles x 8 B (1 bpp, row-major, MSB = left pixel)

Screen directory
  count x 8 B: cell, pad[2], payload_off u24, pad[2]

Screen payloads
  count x 384 B: tiles[192] then attrs[192] (16x12)

Entity types
  count x 10 B: state_count, pad, then up to 4 x (bank, tile_id)

Entity instances
  count x 8 B: type_id, fg, flags(flip_h/v), pad, world_x u16, world_y u16
```

## Attribute byte (on cart and in Nano project)

```text
7 6 5 4 3 2 1 0
| | | | | | |_|__ BANK 0-3
| | | | |_|______ FLIP_H, FLIP_V
| |_|_|__________ FG color 0-7
|________________ SOLID
```

## Studio authoring notes

- Editor still stores CHR as 16-byte 2bpp tiles for the paint UI. Export converts to 1 bpp (any non-zero color becomes FG bit 1).
- Playfield is **128x96** (16x12 tiles).
- RGBS presentation is **2x** to **256x192** (see [`video.md`](video.md)).
- Ctrl+S saves `.r01proj`. Ctrl+E writes `output/nano/test.r01nano` and `output/nano/test_flash.bin`.
