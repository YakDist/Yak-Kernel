#!/usr/bin/env python3

import sys
import subprocess
import os
import re

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

    ret = subprocess.run(args).returncode


    #print("dep file expected at: {}".format(dep_file))

    #start = time.perf_counter()

    if ret == 0 and dep_file and os.path.exists(dep_file):
        fixup_deps(dep_file, sync_dir)

    #print(f"Executed in {time.perf_counter() - start:.3f}s")

    sys.exit(ret)

_CONFIG_PATTERN = re.compile(rb'\bCONFIG_\w+')

def scan_config_strings(filepath):
    with open(filepath, 'rb') as f:
        content = f.read()
    return {m.decode() for m in _CONFIG_PATTERN.findall(content)}

def fixup_deps(dep_filepath, sync_dir):
    with open(dep_filepath, 'r') as f:
        content = f.read()

    # remove comments
    content = re.sub(r'#[^\n]*', '', content)

    content = content.replace('\\\n', ' ')

    tokens = content.split()

    filtered_tokens = [t for t in tokens if "yak/config.h" not in t]
    depends_on_config = len(filtered_tokens) != len(tokens)

    if not filtered_tokens or not depends_on_config:
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
    existing = {e.name for e in os.scandir(sync_dir)}
    deps = []
    for o in all_options:
        name = o[7:]
        if name in existing:
            deps.append(sync_dir + os.sep + name)
        else:
            print(f"WARNING: {o} does not exist!")
    return deps

if __name__ == "__main__":
    main()
