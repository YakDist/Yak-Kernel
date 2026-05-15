#!/bin/bash

before="$1"
after="$2"

rg -F -l "$before" | xargs sd -F "$before" "$after"
