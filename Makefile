OS_NAME := colibri
SYSROOT := sysroot
BOOTDIR := $(SYSROOT)/boot
LIBDIR := $(SYSROOT)/usr/lib
INCLUDEDIR := /usr/include
INCLUDES := -isystem ./libc/include/ -isystem ./kernel/include/

HOST ?= i686-elf
HOSTARCH != ./target-triplet-to-arch.sh $(HOST)

TOOLPREFIX := ../cross/bin/$(HOST)

AR := $(TOOLPREFIX)-ar
AS := $(TOOLPREFIX)-as
CC := $(TOOLPREFIX)-gcc --sysroot=$(SYSROOT) $(INCLUDES) -ggdb -std=c23

.PHONY: all qemu qemu-gdb todo
.SUFFIXES: .libk.o .c .S .o

 all: $(BOOTDIR)/$(OS_NAME).kernel

QEMU_FLAGS := -m 128 -no-reboot -smp 1 -cdrom $(OS_NAME).iso

qemu: $(OS_NAME).iso
	qemu-system-$(HOSTARCH) $(QEMU_FLAGS)

.gdbinit: Makefile
	echo -e 'target remote localhost:1234\nfile $(BOOTDIR)/$(OS_NAME).kernel\n' > .gdbinit

qemu-gdb: $(OS_NAME).iso .gdbinit
	qemu-system-$(HOSTARCH) $(QEMU_FLAGS) -s -S

$(OS_NAME).iso: $(BOOTDIR)/grub/grub.cfg $(BOOTDIR)/$(OS_NAME).kernel
	grub-mkrescue -o $(OS_NAME).iso $(SYSROOT)

$(BOOTDIR)/grub/grub.cfg: Makefile
	mkdir -p $(BOOTDIR)/grub
	echo -e 'menuentry "$(OS_NAME)" {\n\tmultiboot2 /boot/$(OS_NAME).kernel\n}\n' > $(SYSROOT)/boot/grub/grub.cfg

SOURCE_FILES != find libc/ kernel/ -name "*.[c|S|h]"

todo:
	grep -HnF -e TODO -e FIXME $(SOURCE_FILES)

# Making libk (and in the future libc)
include libc/make.config

LIBK_OBJS := $(addprefix libc/, $(LIBK_OBJS))
LIB_HEADERS := $(addprefix libc/, $(LIB_HEADERS))

-include $(LIB_OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)

$(SYSROOT)$(INCLUDEDIR)/%.h:: libc/include/%.h
	@mkdir -p $(@D)
	cp --preserve=timestamps $< $@

$(SYSROOT)$(INCLUDEDIR)/%.h:: kernel/include/%.h
	@mkdir -p $(@D)
	cp --preserve=timestamps $< $@

$(LIBDIR)/libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

%.libk.o: %.c Makefile libc/make.config
	$(CC) -MD -c $< -o $@ $(LIBK_CFLAGS)

install_libc_headers:
	@mkdir -p $(SYSROOT)$(INCLUDEDIR)
	cp --preserve=timestamps -R libc/include/* $(SYSROOT)$(INCLUDEDIR)

# Making the kernel
include kernel/make.config

-include $(KER_OBJS:.o=.d)

$(BOOTDIR)/$(OS_NAME).kernel: $(KER_OBJS) $(KER_ARCHDIR)/linker.ld $(LIBDIR)/libk.a
	$(CC) -MD -T $(KER_ARCHDIR)/linker.ld -o $@ $(KER_CFLAGS) $(KER_LINK_LIST)
	grub-file --is-x86-multiboot2 $(BOOTDIR)/$(OS_NAME).kernel

$(KER_ARCHDIR)/crtbegin.o $(KER_ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(KER_CFLAGS) $(KER_LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

kernel/kernel/font.o:
	cp ../fonts/solarize-12x29-psf/font.o kernel/kernel/font.o

%.o: %.c Makefile kernel/make.config
	$(CC) -MD -c $< -o $@ $(KER_CFLAGS)

%.o: %.S Makefile kernel/make.config
	$(CC) -MD -c $< -o $@ $(KER_CFLAGS)

install_kernel_headers:
	@mkdir -p $(SYSROOT)$(INCLUDEDIR)
	cp --preserve=timestamps -R kernel/include/* $(SYSROOT)$(INCLUDEDIR)/kernel
