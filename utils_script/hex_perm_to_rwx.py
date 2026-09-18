#!/usr/bin/python3

def main():
    scan = input("Permissions: ")
    while scan.strip():
        str_perm = ""
        perm = int(scan, 16)
        # File type
        if (perm & 0xF000) == 0x4000:
            str_perm += "d"
        elif (perm & 0xF000) == 0xA000:
            str_perm += "l"
        else:
            str_perm += "-"

        # User perm
        str_perm += "r" if (perm & 0x100) else "-"
        str_perm += "w" if (perm & 0x80) else "-"
        str_perm += "x" if (perm & 0x40) else "-"
        # Group perm
        str_perm += "r" if (perm & 0x20) else "-"
        str_perm += "w" if (perm & 0x10) else "-"
        str_perm += "x" if (perm & 0x8) else "-"
        # Other perm
        str_perm += "r" if (perm & 0x4) else "-"
        str_perm += "w" if (perm & 0x2) else "-"
        str_perm += "x" if (perm & 0x1) else "-"

        print(f"Permission string is: {str_perm}")

        scan = input("Permissions: ")

if __name__ == "__main__":
    main()
