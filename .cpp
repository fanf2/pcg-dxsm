#!/bin/sh
cat "$@" | cc -E - | sed '/^#/d;/^$/d' | clang-format
