; collision directory: col, row, tab_lo, tab_hi per present screen
        .byte $06, $07, $00, $8E  ; screen (6,7)
        .byte $07, $07, $C0, $8E  ; screen (7,7)
        .byte $08, $07, $80, $8F  ; screen (8,7)
        .byte $08, $08, $40, $90  ; screen (8,8)
play_coll_count: .byte $04
