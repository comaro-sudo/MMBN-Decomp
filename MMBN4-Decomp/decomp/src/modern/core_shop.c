/* Phase C4 / E2 portable C reference implementations of MMBN4 shop stock,
 * pricing, and purchase helpers (docs/re/shops.md, code_03.s).
 *
 * Verified with `make modern-check` and `make modern-test`. */

#include "toolkit.h"

#define SHOP_KIND_ZENNY      0u
#define SHOP_KIND_BUGFRAG    1u
#define SHOP_KIND_CHIP_ORDER 2u

#define SHOP_QTY_UNLIMITED   0xFFu

struct ShopStockEntry {
    u8 type;   /* +0: 0=empty/end, 1=item, 2=chip, 3=NaviCust program */
    u8 qty;    /* +1: quantity left (0xFF = unlimited) */
    u16 id;    /* +2: item/chip/program id */
    u8 code;   /* +4: chip code (26='*'), 0xFF for items, color for programs */
    u8 isNew;  /* +5: chip-order "new" marker */
    u16 price; /* +6: x100 Zenny, or BugFrags as-is in kind-1 shops */
};

/* 0x0804ACF4: ShopPriceToMoney */
u32 ShopPriceToMoney(u32 shopKind, u16 rawPrice) {
    if (shopKind == SHOP_KIND_BUGFRAG)
        return (u32)rawPrice;
    return (u32)rawPrice * 100u;
}

/* 0x0804BC38: ShopGetStock (shop 0..15 -> n*0x40, shop 16 -> 0x400) */
struct ShopStockEntry *ShopGetStock(struct Toolkit *tk, u8 shopIndex) {
    u32 byteOff = shopIndex >= 16 ? 0x400u : ((u32)shopIndex * 0x40u);
    return (struct ShopStockEntry *)(tk->shopStock + byteOff);
}

/* 0x0804BB34: ShopHasStock */
int ShopHasStock(const struct ShopStockEntry *slots, u32 slotCount) {
    u32 i;
    for (i = 0; i < slotCount; i++) {
        if (slots[i].type == 0)
            break;
        if (slots[i].qty != 0)
            return 1;
    }
    return 0;
}

/* 0x0804BC4C: ShopTakeStock (decrements qty unless 0xFF; compacts when 0 if shiftUp) */
int ShopTakeStock(
    struct ShopStockEntry *slots,
    u32 slotCount,
    u8 type,
    u16 id,
    u8 code,
    int shiftUp
) {
    u32 i;
    for (i = 0; i < slotCount; i++) {
        if (slots[i].type == 0)
            break;
        if (slots[i].type == type && slots[i].id == id && slots[i].code == code) {
            if (slots[i].qty == SHOP_QTY_UNLIMITED)
                return 1;
            if (slots[i].qty == 0)
                return 0;
            slots[i].qty--;
            if (slots[i].qty == 0 && shiftUp) {
                u32 j;
                for (j = i; j + 1 < slotCount; j++)
                    slots[j] = slots[j + 1];
                slots[slotCount - 1].type = 0;
                slots[slotCount - 1].qty = 0;
            }
            return 1;
        }
    }
    return 0;
}

/* 0x0804BBFC: ShopRestockItem (+1 qty, cap 255) */
int ShopRestockItem(struct ShopStockEntry *slots, u32 slotCount, u16 id) {
    u32 i;
    for (i = 0; i < slotCount; i++) {
        if (slots[i].type == 0)
            break;
        if (slots[i].id == id) {
            if (slots[i].qty < 0xFFu)
                slots[i].qty++;
            return 1;
        }
    }
    return 0;
}
