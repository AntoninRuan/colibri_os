#!/usr/bin/python3

def main():
    scan = input("GDT entry: ")
    while scan.strip():
        entry = int(scan, 16)

        flags = (entry >> 52) & 0xF
        access_byte = (entry >> 40) & 0xFF
        base_high = (entry >> 56) & 0xFF
        base_med = (entry >> 32) & 0xFF
        base_low = (entry >> 16) & 0xFFFF
        limit_high = (entry >> 48) & 0xF
        limit_low = (entry >> 0) & 0xFFFF

        base = (base_high << 24) | (base_med << 16) | base_low
        limit = (limit_high << 16) | limit_low

        flags_readable = []
        if flags & 0b1000:
            flags_readable.append("G")
        if flags & 0b0100:
            flags_readable.append("DB")
        if flags & 0b0010:
            flags_readable.append("L")

        access = []
        if access_byte & 0b1000_0000:
            access.append("P")
        access.append(f"DPL({(access_byte >> 5) & 0b11})")
        if access_byte & 0b0001_0000:
            access.append("S")
        if access_byte & 0b0000_1000:
            access.append("E")
        if access_byte & 0b0000_0100:
            access.append("DC")
        if access_byte & 0b0000_0010:
            access.append("RW")
        if access_byte & 0b0000_0001:
            access.append("A")

        print(f"flags ={flags_readable}")
        print(f"access={access}")
        print(f"base  ={hex(base)}")
        print(f"limit ={hex(limit)}")

        scan = input("GDT entry: ")


if __name__ == "__main__":
    main()
