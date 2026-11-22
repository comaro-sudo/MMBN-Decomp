# MMBN4 disassembly — build and byte-compare against the original ROM.
#
#   make                 # VERSION=rs (US Red Sun, B4WE)
#   make VERSION=bm      # US Blue Moon (B4BE) — enabled at DC-4
#   make compare         # build + SHA-1 check (must match)
#   make clean
#
# Runs in WSL (Ubuntu): needs binutils-arm-none-eabi, make, sha1sum.
# ROM bytes are only read from ../roms/ at build time (never committed).

VERSION ?= rs

ifeq ($(VERSION),rs)
  BASEROM  := ../roms/bn4_rs_us.gba
  SHA1     := a97e96a7da03abd70f7953328e511b9fa29179f1
else ifeq ($(VERSION),bm)
  BASEROM  := ../roms/bn4_bm_us.gba
  SHA1     := 5e017c803ddc768efb8010314b46bc17e9757f71
else
  $(error VERSION must be rs or bm)
endif

PREFIX  := arm-none-eabi-
AS      := $(PREFIX)as
LD      := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy

BUILD   := build/$(VERSION)
ROM     := $(BUILD)/bn4_$(VERSION).gba
ELF     := $(BUILD)/bn4_$(VERSION).elf
OBJ     := $(BUILD)/rom.o

ASFLAGS := -mcpu=arm7tdmi -mthumb-interwork -I $(BUILD) -I include -I . \
           --defsym VERSION_RS=$(if $(filter rs,$(VERSION)),1,0) \
           --defsym VERSION_BM=$(if $(filter bm,$(VERSION)),1,0)

.PHONY: all compare clean
all: $(ROM)

# baserom.gba inside the build dir, so .incbin "baserom.gba" finds it via -I.
$(BUILD)/baserom.gba: $(BASEROM)
	@mkdir -p $(BUILD)
	@echo "$(SHA1)  $<" | sha1sum -c --quiet - || (echo "baserom $< has the wrong SHA-1"; exit 1)
	ln -sf $(abspath $<) $@

$(OBJ): rom.s $(wildcard asm/*.s) $(wildcard data/*.s) $(wildcard include/*.inc) $(BUILD)/baserom.gba
	$(AS) $(ASFLAGS) -o $@ rom.s

$(ELF): $(OBJ) ld_script.ld
	$(LD) -T ld_script.ld -Map $(BUILD)/bn4_$(VERSION).map -o $@ $(OBJ)

$(ROM): $(ELF)
	$(OBJCOPY) -O binary $< $@

compare: $(ROM)
	@echo "$(SHA1)  $(ROM)" | sha1sum -c -

clean:
	rm -rf build
