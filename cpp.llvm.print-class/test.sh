#!/bin/bash

ninja -C ./build

# form A: separate plugin arg token (no leading dash)
clang -cc1 -load ./build/libprint-class-plugin.so -plugin print-class \
      -plugin-arg-print-class class=A ./test.cpp

