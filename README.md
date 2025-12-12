# List Scheduling playground
Playground to implement list-scheduling in C++.

## Overview

The current implementation changed towards scheduling individual stages (i.e. finding a feasible schedule for the used stages/substages). Nodes for the needed pipeline stages of instructions are added iteratively.

The preious implementations 
- branch hls_example: is based on List-Scheduling for high-level synthesis (2x MUL / 1x ALU) functional units.
- branch instr_execute_scheduling: only schedules the functional units of the execute stage

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

The schedule (in simple_test) is calculated for individual stages dependecies between instructions.

Priorities are based on tuple: 
    
    (operands_ready, instr_idx, node_id)

# License

The code in this repository is released under the MIT License. Use of this source code is governed by a MIT-style license that can be found in the LICENSE file.