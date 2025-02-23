#!/usr/bin/python3

def main():
    scan = input("Virtual Address: ")
    while scan.strip():
        physical_addr = int(scan, 16)

        if (physical_addr >> 47) != 0x1FFFF and (physical_addr >> 47) != 0:
            print("WARNING: Address is not canonical")

        pml4_entry = (physical_addr >> 39) & 0x1FF
        pdpr_entry = (physical_addr >> 30) & 0x1FF
        pd_entry = (physical_addr >> 21) & 0x1FF
        pt_entry = (physical_addr >> 12) & 0x1FF
        offset_small = (physical_addr) & 0xFFF
        offset_big = physical_addr & 0x1FFFFF

        print(f"Address {hex(physical_addr)} translates to:")

        print(f"pml4 entry={pml4_entry}")
        print(f"pdpr entry={pdpr_entry}")
        print(f"pd   entry={pd_entry}")
        print(f"For 2M pages:")
        print(f"     offset  ={offset_big}")
        print(f"For 4K pages:")
        print(f"     pt_entry={pt_entry}")
        print(f"     offset  ={offset_small}")

        scan = input("Virtual Address: ")

if __name__ == "__main__":
    main()
