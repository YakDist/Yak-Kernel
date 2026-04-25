#!/usr/bin/env python3

import sys
import subprocess
import os
import re
import time

def main():
    sync_dir = sys.argv[1]
    args = sys.argv[2:]

    #print("fixdep.py is running: {}".format(args))

    # locate -MF argument
    dep_file = None
    try:
        mf_index = args.index("-MF")
        dep_file = args[mf_index + 1]
    except (ValueError, IndexError):
        # just run normally if no -MF was passed
        pass

    result = subprocess.run(args)


    #print("dep file expected at: {}".format(dep_file))

    start = time.perf_counter()

    if result.returncode == 0:
        if dep_file and os.path.exists(dep_file):
            fixup_deps(dep_file, sync_dir)

    print(f"Executed in {time.perf_counter() - start:.3f}s")

    sys.exit(result.returncode)

def scan_config_strings(filepath):
    pattern = re.compile(r'\bCONFIG_\w+')
    with open(filepath, 'r') as f:
        content = f.read()
    return set(pattern.findall(content))

def fixup_deps(dep_filepath, sync_dir):
    with open(dep_filepath, 'r') as f:
        content = f.read()

    # remove comments
    content = '\n'.join(line for line in content.split('\n') if '#' not in line)

    content = content.replace('\\\n', ' ')

    tokens = content.split()

    filtered_tokens = [t for t in tokens if "yak/config.h" not in t]

    if not filtered_tokens:
        return

    target = filtered_tokens[0]
    deps = filtered_tokens[1:]

    # linux' fixdep.c has some special casing for DT and rustc sources. If needed, the first source dep following the colon (deps[1]) is normally the source file.

    all_options = set().union(*[scan_config_strings(d) for d in deps])
    # slice off the CONFIG_ prefix
    sync_deps = get_sync_deps(sync_dir, all_options)
    deps += sync_deps

    phony_rules = "".join(f"\n{dep}:\n" for dep in sync_deps)
    new_content = f"{target}\\\n  " + " \\\n  ".join(deps) + "\n" + phony_rules

    with open(dep_filepath, 'w') as f:
        f.write(new_content)

def get_sync_deps(sync_dir, all_options):
    deps = []
    for o in all_options:
        p = sync_dir + os.sep + o[7:]
        if os.path.exists(p):
            deps.append(p)
        else:
            print(f"WARNING: {o} does not exist!")
    return deps

if __name__ == "__main__":
    main()
