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
