#!/usr/bin/python3

import sys
import subprocess

def main():
    failed_list = []
    total_test = 0
    failed = 0
    for test in sys.argv[1:]:
        print(f"Testing {test}")
        p = subprocess.run([f"./{test}"], capture_output=True)
        out = p.stdout.decode("utf-8")
        print(out, end="")
        total_test += out.count("\n")
        failed += p.returncode
        if p.returncode > 0:
            failed_list.append(test)

    success = total_test - failed
    percent = int((success / total_test) * 100)

    s_str = f"\33[32m{success}\33[0m"
    f_str = f"\33[31m{failed}\33[0m"
    p_str = ""
    if (percent > 85):
        p_str = f"\33[32m{percent}\33[0m"
    elif (percent > 60):
        p_str = f"\33[33m{percent}\33[0m"
    else:
        p_str = f"\33[31m{percent}\33[0m"

    print(f"{total_test} Ran, {s_str} Passed, {f_str} Failed. {p_str} % Success rate")

if __name__ == "__main__":
    main()
