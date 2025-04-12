#!/usr/bin/python3
import numpy as np
import matplotlib.pyplot as plt

def calculate_overhead(size, GS=4096):
    nsizes = int(np.log2(size/GS)) + 1

    if size > (1 << (nsizes - 1)) * GS:
        nsizes += 1

    overhead = 0
    overhead += 8 * 3

    for k in range(nsizes):
        overhead += 1 * int(np.ceil((2<<(nsizes-1 - k)) / 8))

    for k in range(1, nsizes):
        overhead += 1 * int(np.ceil((2<<(nsizes-1 - k)) / 8))

    overhead += max(GS - (overhead % GS), 0)
    return overhead * 100 / size

def main():
    GS = 4096

    sizes = np.logspace(13, 23, base=2, num=100, dtype=np.uint64)

    voverhead = np.vectorize(calculate_overhead)
    overheads_pct = voverhead(sizes)

    fig, ax = plt.subplots()
    # ax.set_xscale('log', base=2)
    # ax.xaxis.set_ticks(np.logspace(13,23,base=2,num=(23-13+1), dtype=np.uint64))
    ax.plot(sizes, overheads_pct)
    plt.show()

if __name__ == "__main__":
    main()
