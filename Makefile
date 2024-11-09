OS_NAME := colibri
SYSROOT := sysroot
BUILD   := build/
BOOTDIR := $(SYSROOT)/boot
LIBDIR := $(SYSROOT)/usr/lib

HOST ?= i686-pc-none-elf
HOSTARCH_QEMU != ./target-triplet-to-arch.sh $(HOST)
HOSTARCH := $(subst _,-,$(HOSTARCH_QEMU))

LLVM_TARGET_FLAG := --target=$(HOST) -march=$(HOSTARCH)

AR := llvm-ar
CC := clang $(LLVM_TARGET_FLAG) -ggdb -std=gnu23

.PHONY: all qemu qemu-gdb todo clean
.SUFFIXES: .libk.o .c .S .o

all: $(BOOTDIR)/$(OS_NAME).kernel

clean:
	rm -R $(BUILD)
	rm -R $(SYSROOT)

CPUS ?= 1

QEMU_FLAGS := -m 128 -no-reboot -smp $(CPUS) -cdrom $(OS_NAME).iso

.gdbinit: Makefile
	echo -e 'target remote localhost:1234\nfile $(BOOTDIR)/$(OS_NAME).kernel\n' > .gdbinit

qemu: $(OS_NAME).iso
	qemu-system-$(HOSTARCH_QEMU) $(QEMU_FLAGS)

qemu-gdb: $(OS_NAME).iso .gdbinit
	qemu-system-$(HOSTARCH_QEMU) $(QEMU_FLAGS) -s -S

$(OS_NAME).iso: $(BOOTDIR)/grub/grub.cfg $(BOOTDIR)/$(OS_NAME).kernel
	grub-mkrescue -o $(OS_NAME).iso $(SYSROOT)

$(BOOTDIR)/grub/grub.cfg: Makefile
	@mkdir -p $(BOOTDIR)/grub
	echo -e 'menuentry "$(OS_NAME)" {\n\tmultiboot2 /boot/$(OS_NAME).kernel\n}\n' > $(SYSROOT)/boot/grub/grub.cfg

SOURCE_FILES != find libc/ kernel/ -name "*.[c|S|h]"

todo:
	grep -HnF -e TODO -e FIXME $(SOURCE_FILES)

# Making libk (and in the future libc)
include libc/make.config

LIBK_OBJS := $(addprefix $(BUILD)libc/, $(LIBK_OBJS))
LIB_HEADERS := $(addprefix $(BUILD)libc/, $(LIB_HEADERS))

-include $(LIB_OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)

$(LIBDIR)/libk.a: $(LIBK_OBJS)
	@mkdir -p $(LIBDIR)
	$(AR) rcs $@ $(LIBK_OBJS)

$(BUILD)%.libk.o: %.c Makefile libc/make.config
	@mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS)

# Making the kernel
include kernel/make.config

KER_OBJS := $(addprefix $(BUILD), $(KER_OBJS))

-include $(KER_OBJS:.o=.d)

$(BOOTDIR)/$(OS_NAME).kernel: $(KER_OBJS) $(KER_ARCHDIR)/linker.ld $(LIBDIR)/libk.a
	@mkdir -p $(BOOTDIR)
	ld.lld -n -T $(KER_ARCHDIR)/linker.ld -o $@ $(KER_LINK_LIST) -L$(LIBDIR) --Map $(BUILD)kernel.map -static
	grub-file --is-x86-multiboot2 $(BOOTDIR)/$(OS_NAME).kernel

$(BUILD)%.o: %.c Makefile kernel/make.config
	@mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ $(KER_CFLAGS)

$(BUILD)%.o: %.S Makefile kernel/make.config
	@mkdir -p $(@D)
	$(CC) -MD -c $< -o $@ $(KER_CFLAGS)
