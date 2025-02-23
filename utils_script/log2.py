#!/usr/bin/python

def sup_log2(x: int) -> int:
    if (x == 0):
        return 0
    if (x == 1):
        return 0
    result = 0
    sup = False
    while (x > 1):
        if (x & 1):
            sup = True
        x = x >> 1
        result += 1
    if sup:
        result += 1
    return result

def main():
    print(f"sup_log2={sup_log2(0xFFFFFFFFFFFFFFFF)}")

if __name__ == "__main__":
    main()
