# List Scheduling playground
Playground to implement list-scheduling in C++.

## Overview

The current implementation changed towards scheduling instructions into individual functional units (i.e. finding a feasible schedule for a specific instruction trace). Instruction-nodes are added iteratively.
To speedup calculation, each instruction starts from its earliest possibile issue time.
If any previous instruction starts using the same functional units at a later time, we need to reschedule (starting from this instruction).

The intial implementation is based on List-Scheduling for high-level synthesis (2x MUL / 1x ALU) functional units.
(see Branch hls_example)

## Usage

### Build

Run following command from root folder:

    make all

### Running the simple_test

Either:

    make run

or:

    ./simple_test

### simple_test Example

The schedule is calculated for following dependency graph:
Priorities are based on tuple: 
    
    (operands_ready, issue_ready, id)

# License

The code in this repository is released under the MIT License. Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.