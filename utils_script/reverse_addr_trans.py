#!/usr/bin/python

def main():
    pml4_offset = int(input("pml4 offset: "))
    pdpt_offset = int(input("pdpt offset: "))
    pd_offset = int(input("pd offset: "))
    pt_offset = input("pt offset: ")
    big_page = not(pt_offset.strip())
    if not big_page:
        pt_offset = int(pt_offset)
    offset = int(input("offset: "))

    addr = 0
    if pml4_offset & 0x100:
        addr |= 0xFFFF << 48

    addr |= pml4_offset << 39
    addr |= pdpt_offset << 30
    addr |= pd_offset << 21
    if not big_page:
        addr |= pt_offset << 12
    addr |= offset

    print(f"Address is {hex(addr)}")

if __name__ == "__main__":
    main()
