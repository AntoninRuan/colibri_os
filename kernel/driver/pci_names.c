#include <kernel/driver/pci.h>

static char *pci_subclass_0x0[2] = {
    "Non-VGA-compatible unclassified device",
    "VGA-compatible unclassified device",
};

static char *pci_subclass_0x1[] = {
    "SCSI bus controller",    "IDE controller",
    "Floppy Disk controller", "IPI bus controller",
    "RAID controller",        "ATA controller",
    "SATA controller",        "Serial Attached SCSI Controller",
    "NVM controller",         [0x80] = "Other mass storage controller",
};

static char *pci_subclass_0x2[] = {
    "Ethernet controller",
    "Token ring controller",
    "FDDI controller",
    "ATM controller",
    "ISDN controller",
    "WorldFip controller",
    "PICMG 2.14 Multi computing controller",
    "Infiniband controller",
    "Fabric controller",
    [0x80] = "Other network controller",
};

static char *pci_subclass_0x3[] = {
    "VGA compatible controller",
    "XGA controller",
    "3D controller",
    [0x80] = "Other display controller",
};

static char *pci_subclass_0x4[] = {
    "Multimedia video controller",      "Multimedia audio controller",
    "Computer telephony deivce",        "Audio device",
    [0x80] = "Other multimedia device",
};

static char *pci_subclass_0x5[] = {
    "RAM controller",
    "Flash controller",
    [0x80] = "Other memory controller",
};

static char *pci_subclass_0x6[] = {
    "Host bridge",
    "ISA bridge",
    "EISA bridge",
    "MCA bridge",
    "PCI-to-PCI bridge",
    "PCMCIA bridge",
    "Nubus bridge",
    "Cardbus bridge",
    "RACEway bridge",
    "PCI-to-PCI bridge",
    "Infiniband-to-PCI Host bridge",
    [0x80] = "Other bridge device",
};

static char *pci_subclass_0x7[] = {
    "Serial controller",
    "Paraller controller",
    "Multiport serial controller",
    "Modem",
    "IEEE 488.1/2 controller",
    "Smart card controller",
    [0x80] = "Other communication device",
};

static char *pci_subclass_0x8[] = {
    "PIC",
    "DMA controller",
    "Timer",
    "RTC controller",
    "PCI Hot-Plug controller",
    "SD host controller",
    "IOMMU",
    [0x80] = "Other system peripheral",
};

static char *pci_subclass_0x9[] = {
    "Keyboard controller", "Digitizer pen",
    "Mouse controller",    "Scanner controller",
    "Gameport controller", [0x80] = "Other input controller",
};

static char *pci_subclass_0xA[] = {
    "Docking station: Generic",
    [0x80] = "Other type of docking station",
};

static char *pci_subclass_0xB[] = {
    "i386",
    "i486",
    "Pentium",
    "Pentium Pro",
    [0x10] = "Alpha",
    [0x20] = "Power PC",
    [0x30] = "MIPS",
    [0x40] = "Co-Processor",
    [0x80] = "Other processors",
};

static char *pci_subclass_0xC[] = {
    "Firewire controller",
    "ACCESS bus controller",
    "SSA",
    "USB controller",
    "Fibre Channel",
    "SMBus controller",
    "Infiniband controller",
    "IPMI interface",
    "SERCOS interface",
    "CANbus controller",
    [0x80] = "Other serial bus controller",
};

static char *pci_subclass_0xD[] = {
    "iRDA compatible controller",
    "Consumer IR controller",
    [0x10] = "RF controller",
    [0x11] = "Bluetooth controller",
    [0x12] = "Broadband controller",
    [0x20] = "Ethernet controller (802.1a)",
    [0x21] = "Ethernet controller (802.1b)",
    [0x80] = "Other type of wireless controller",
};

static char *pci_subclass_0xE[] = {"I2O"};

static char *pci_subclass_0xF[] = {
    [0x1] = "Satellite TV controller",
    [0x2] = "Satellite audio controller",
    [0x3] = "Satellite voice controller",
    [0x4] = "Satellite data controller",
    [0x80] = "Other satellite communication controller",
};

static char *pci_subclass_0x10[] = {
    "Network and Computing encryption/decryption",
    [0x10] = "Entertainment encryption/decryption",
    [0x80] = "Other encryption/decryption controller",
};

static char *pci_subclass_0x11[] = {
    "DPIO modules",
    "Performance counters",
    [0x10] = "Communication synchronizer",
    [0x20] = "Management card",
    [0x80] = "Other data acquisition/signal processing controller",
};

static char *pci_subclass_0x12[] = {
    "Processing accelerator",
};

static char *pci_subclass_0x13[] = {
    "Non essential instrumentation function"
};

char **pci_class_name[] = {
    pci_subclass_0x0,
    pci_subclass_0x1,
    pci_subclass_0x2,
    pci_subclass_0x3,
    pci_subclass_0x4,
    pci_subclass_0x5,
    pci_subclass_0x6,
    pci_subclass_0x7,
    pci_subclass_0x8,
    pci_subclass_0x9,
    pci_subclass_0xA,
    pci_subclass_0xB,
    pci_subclass_0xC,
    pci_subclass_0xD,
    pci_subclass_0xE,
    pci_subclass_0xF,
    pci_subclass_0x10,
    pci_subclass_0x11,
    pci_subclass_0x12,
    pci_subclass_0x13,
};
