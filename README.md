# LC3-like VM

A Virtual Machine implementation in C to mimic LC3.
Based on: https://justinmeiners.github.io/lc3-vm/

## How to test

1. `make lc3-vm` will generate the executable.
2. `./lc3-vm 2080.obj` or `./lc3-vm rogue.obj`.
  - the script takes an `.ojb` file (a simple program that can run on LC3) as an argument.
