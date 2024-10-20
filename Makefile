SYSROOT := sysroot
BOOTDIR := $(SYSROOT)/boot
LIBDIR := $(SYSROOT)/usr/lib
INCLUDEDIR := /usr/include

HOST ?= i686-elf
HOSTARCH != ./target-triplet-to-arch.sh $(HOST)

TOOLPREFIX := ../cross/bin/$(HOST)

AR := $(TOOLPREFIX)-ar
AS := $(TOOLPREFIX)-as
CC := $(TOOLPREFIX)-gcc --sysroot=$(SYSROOT) -isystem=$(INCLUDEDIR)

.PHONY: all qemu todo
.SUFFIXES: .libk.o .c .S .o

# Qemu
qemu: wos.iso
	qemu-system-$(HOSTARCH) -cdrom wos.iso

all: $(BOOTDIR)/wos.kernel

wos.iso: $(BOOTDIR)/grub/grub.cfg $(BOOTDIR)/wos.kernel
	grub-mkrescue -o wos.iso $(SYSROOT)

$(BOOTDIR)/grub/grub.cfg: Makefile
	mkdir -p $(BOOTDIR)/grub
	echo -e 'menuentry "wOS" {\n\tmultiboot2 /boot/wos.kernel\n}\n' > $(SYSROOT)/boot/grub/grub.cfg

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
	$(CC) -MD -c $< -o $@ -std=gnu11 $(LIBK_CFLAGS)

# Making the kernel
include kernel/make.config

-include $(KER_OBJS:.o=.d)

$(BOOTDIR)/wos.kernel: $(KER_OBJS) $(KER_ARCHDIR)/linker.ld $(LIBDIR)/libk.a
	$(CC) -MD -T $(KER_ARCHDIR)/linker.ld -o $@ $(KER_CFLAGS) $(KER_LINK_LIST)
	grub-file --is-x86-multiboot2 $(BOOTDIR)/wos.kernel

$(KER_ARCHDIR)/crtbegin.o $(KER_ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(KER_CFLAGS) $(KER_LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

%.o: %.c Makefile kernel/make.config
	$(CC) -MD -c $< -o $@ -std=gnu11 $(KER_CFLAGS)

%.o: %.S Makefile kernel/make.config
	$(CC) -MD -c $< -o $@ -std=gnu11 $(KER_CFLAGS)
