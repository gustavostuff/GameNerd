; collision directory: col, row, tab_lo, tab_hi per present screen
        .byte $00, $00, $00, $8E  ; screen (0,0)
        .byte $01, $00, $C0, $8E  ; screen (1,0)
        .byte $02, $00, $80, $8F  ; screen (2,0)
        .byte $00, $01, $40, $90  ; screen (0,1)
        .byte $01, $01, $00, $91  ; screen (1,1)
        .byte $02, $01, $C0, $91  ; screen (2,1)
        .byte $00, $02, $80, $92  ; screen (0,2)
        .byte $01, $02, $40, $93  ; screen (1,2)
        .byte $02, $02, $00, $94  ; screen (2,2)
play_coll_count: .byte $09
