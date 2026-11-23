; Hello-world test patch (M0). Not part of the overhaul.
; Cannon damage 40 -> 99. Visible in the Folder/Library and in battle.
; Enable with: python patch/build.py --hello

chip_field_org CHIP_CANNON, CHIP_OFS_DAMAGE
    .halfword 99
