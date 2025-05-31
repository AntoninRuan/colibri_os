#!/usr/bin/python3
import subprocess
from tqdm import tqdm
import numpy as np

tree_sizes = [1000, 3_000, 10_000, 30_000]
ops = 10_000_000

def main():
    size = tree_sizes[0]
    for size in tree_sizes:
        print(f"Testing for size {size}")
        results = []
        for i in tqdm(range(10)):
            p = subprocess.run(["./rb_benchmark", str(size), str(ops)], capture_output=True)
            out = p.stdout.decode("utf-8")
        results.append(out.split("\n")[-2])
        result_numeric = [float(x) for x in results]
        m = np.mean(result_numeric)
        indiv_nano = (m * 10**(9)) / ops
        print(f"Took in average {m:.2f}s to run ({indiv_nano:.0f}ns/op)")

if __name__ == "__main__":
    main()
