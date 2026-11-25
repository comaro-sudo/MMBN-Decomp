; ============================================================
; WP-A3 (plan WP-A4): Key-item & upgrade shop price reduction — Option 2 (owner decision 2026-11-24).
; Research: Gemini, decomp/notes/key-item-prices.md (round 12); docs/re/shops.md.
;
; Cuts 34 shop slots in BN4_SHOP_STOCK_INIT (RS 0x0804ADA0 / BM 0x0804ADA8, 8B/slot, u16 price at +6
; in units of 100 Zenny) to 40% of their vanilla price (60% off):
;   - 14 HP Memory slots (Shops 0, 1, 2, 3, 5):      207,700Z -> 83,000Z
;   - 12 Zenny NaviCust slots (Shops 14-15):          92,900Z -> 36,900Z
;   - 4 Unlocker slots (Shops 6, 9, 10, 11):           4,000Z ->  1,600Z
;   - 4 LocEnemy slots (Shops 6, 7, 9, 10):           10,000Z ->  4,000Z
;
; Note: BN4_SHOP_STOCK_INIT is copied to the save at New Game (ShopStockInit), so existing saves
; keep their old prices until a load-time sync hook is added.
; ============================================================

.macro shop_price, shop_idx, slot_idx, raw_price
    .org SHOP_STOCK_INIT + (shop_idx) * 0x40 + (slot_idx) * 8 + 6
    .halfword raw_price
.endmacro

; ---- 1. HP Memory (14 slots: 207,700Z -> 83,000Z) ----
; Shop 5: ACDC Area 2
shop_price  5, 0,   4               ;  1,000Z ->    400Z
shop_price  5, 1,  13               ;  3,200Z ->  1,300Z
; Shop 0: Town Area 3
shop_price  0, 0,  10               ;  2,500Z ->  1,000Z
shop_price  0, 1,  24               ;  6,000Z ->  2,400Z
shop_price  0, 2,  40               ; 10,000Z ->  4,000Z
; Shop 1: Netopia Area
shop_price  1, 0,  24               ;  6,000Z ->  2,400Z
shop_price  1, 1,  40               ; 10,000Z ->  4,000Z
shop_price  1, 2,  56               ; 14,000Z ->  5,600Z
; Shop 2: Undernet 1
shop_price  2, 0,  40               ; 10,000Z ->  4,000Z
shop_price  2, 1,  60               ; 15,000Z ->  6,000Z
shop_price  2, 2,  80               ; 20,000Z ->  8,000Z
shop_price  2, 3, 120               ; 30,000Z -> 12,000Z
; Shop 3: Undernet 4
shop_price  3, 0, 120               ; 30,000Z -> 12,000Z
shop_price  3, 1, 200               ; 50,000Z -> 20,000Z

; ---- 2. Zenny NaviCust Programs (12 slots: 92,900Z -> 36,900Z) ----
; Shop 14: Castillo HP
shop_price 14, 0,  14               ; HP+100   (Green)   3,400Z -> 1,400Z
shop_price 14, 1,  19               ; ChargMAX (Yellow)  4,800Z -> 1,900Z
shop_price 14, 2,  22               ; Shield   (Red)     5,500Z -> 2,200Z
shop_price 14, 3,  27               ; FstBarr  (Pink)    6,800Z -> 2,700Z
shop_price 14, 4,  32               ; MegFldr1 (White)   8,000Z -> 3,200Z
shop_price 14, 5,  38               ; Custom1  (Red)     9,600Z -> 3,800Z
; Shop 15: Undernet 6
shop_price 15, 0,  38               ; HP+300   (Yellow)  9,600Z -> 3,800Z
shop_price 15, 1,  25               ; OilBody  (Blue)    6,200Z -> 2,500Z
shop_price 15, 2,  32               ; MegFldr1 (White)   8,000Z -> 3,200Z
shop_price 15, 3,  38               ; Custom1  (Red)     9,600Z -> 3,800Z
shop_price 15, 4,  34               ; FlotShoe (White)   8,400Z -> 3,400Z
shop_price 15, 5,  52               ; Reflect  (Red)    13,000Z -> 5,200Z

; ---- 3. SubChips: Unlocker (4 slots: 4,000Z -> 1,600Z) & LocEnemy (4 slots: 10,000Z -> 4,000Z) ----
shop_price  6, 1,  40               ; Park Area 1   LocEnemy 10,000Z -> 4,000Z
shop_price  6, 2,  16               ; Park Area 1   Unlocker  4,000Z -> 1,600Z
shop_price  7, 2,  40               ; ACDC Town     LocEnemy 10,000Z -> 4,000Z
shop_price  9, 1,  40               ; MelSquar      LocEnemy 10,000Z -> 4,000Z
shop_price  9, 2,  16               ; MelSquar      Unlocker  4,000Z -> 1,600Z
shop_price 10, 1,  16               ; ColosseumAve  Unlocker  4,000Z -> 1,600Z
shop_price 10, 2,  40               ; ColosseumAve  LocEnemy 10,000Z -> 4,000Z
shop_price 11, 2,  16               ; YumRuins      Unlocker  4,000Z -> 1,600Z

; ---- 4. Existing-save load hook (SaveLoad -> SaveRelocateBlock) ----
; Syncs pre-existing saves' shop stock ([sl+0x58], shops 0..15) to WP-A3's 40% prices on load.
; Completely stateless and idempotent:
;   - Unlocker (kind=1, id=181) -> 16 (1,600Z)
;   - LocEnemy (kind=1, id=180) -> 40 (4,000Z)
;   - Zenny NaviCust (shops 14-15, kind=3) -> copy price from SHOP_STOCK_INIT[shop] matching id
;   - HPMemory (kind=1, id=160) -> match (shop, vanilla_price) in 14-entry table (disjoint per shop)

.org SAVE_LOAD_RELOC_HOOK           ; RS 0x08048948 / BM 0x08048950 (bl 0x08006128)
    bl      VENEER_SAVE_LOAD

.org VENEER_SAVE_LOAD
.align 4
    ldr     r3, =save_load_sync_shops | 1
    bx      r3
    .pool

.org FS_SAVE_LOAD_SYNC
.align 4
save_load_sync_shops:
    push    r4-r7, r14
    ldr     r3, =0x08006128 | 1     ; SaveRelocateBlock (RS == BM): rebuilds [sl+0x58]
    bl      @@call_r3
    cmp     r4, 0                   ; r4 == 0 -> valid save loaded
    bne     @@done
    mov     r0, r10
    ldr     r5, [r0, 0x58]          ; r5 = save shop stock buffer (shops 0..15)
    mov     r6, 0                   ; r6 = flat slot index 0..127
@@slot_loop:
    lsl     r7, r6, 3
    add     r7, r5, r7              ; r7 = &save_shop[r6]
    ldrb    r0, [r7, 0]             ; kind
    cmp     r0, 0
    beq     @@next_slot
    ldrh    r1, [r7, 2]             ; item/program id
    cmp     r0, 1
    beq     @@is_item
    cmp     r0, 3
    bne     @@next_slot
    ; NaviCust program: only Zenny shops 14-15
    lsr     r2, r6, 3               ; shop_idx
    cmp     r2, 14
    blt     @@next_slot
    lsl     r2, r2, 6               ; shop_idx * 0x40
    ldr     r3, =SHOP_STOCK_INIT
    add     r3, r3, r2              ; &SHOP_STOCK_INIT[shop][0]
    mov     r4, 0
@@nc_loop:
    ldrh    r0, [r3, 2]
    cmp     r0, r1
    bne     @@nc_next
    ldrh    r0, [r3, 6]             ; copy patched ROM price
    strh    r0, [r7, 6]
    b       @@next_slot
@@nc_next:
    add     r3, 8
    add     r4, 1
    cmp     r4, 8
    blt     @@nc_loop
    b       @@next_slot
@@is_item:
    cmp     r1, 181                 ; Unlocker -> 16 (1,600Z)
    bne     @@chk_loc
    mov     r0, 16
    strh    r0, [r7, 6]
    b       @@next_slot
@@chk_loc:
    cmp     r1, 180                 ; LocEnemy -> 40 (4,000Z)
    bne     @@chk_hp
    mov     r0, 40
    strh    r0, [r7, 6]
    b       @@next_slot
@@chk_hp:
    cmp     r1, 160                 ; HPMemory
    bne     @@next_slot
    lsr     r2, r6, 3               ; r2 = shop_idx
    ldrh    r1, [r7, 6]             ; r1 = current price
    ldr     r3, =@@hp_map
@@hp_loop:
    ldrh    r0, [r3, 0]             ; entry shop (0xFFFF = end)
    cmp     r0, 0xFF
    bgt     @@next_slot
    cmp     r0, r2
    bne     @@hp_next
    ldrh    r0, [r3, 2]             ; vanilla price
    cmp     r0, r1
    bne     @@hp_next
    ldrh    r0, [r3, 4]             ; patched 40% price
    strh    r0, [r7, 6]
    b       @@next_slot
@@hp_next:
    add     r3, 6
    b       @@hp_loop
@@next_slot:
    add     r6, 1
    cmp     r6, 128
    blt     @@slot_loop
@@done:
    pop     r4-r7, r15
@@call_r3:
    bx      r3
    .pool

.align 2
@@hp_map:
    .halfword 5,  10,   4
    .halfword 5,  32,  13
    .halfword 0,  25,  10
    .halfword 0,  60,  24
    .halfword 0, 100,  40
    .halfword 1,  60,  24
    .halfword 1, 100,  40
    .halfword 1, 140,  56
    .halfword 2, 100,  40
    .halfword 2, 150,  60
    .halfword 2, 200,  80
    .halfword 2, 300, 120
    .halfword 3, 300, 120
    .halfword 3, 500, 200
    .halfword 0xFFFF

.if . > FS_SAVE_LOAD_SYNC + FS_SAVE_LOAD_SYNC_SIZE
    .error "WP-A3 save-load shop sync: overruns its free-space slot"
.endif
