# List Scheduling playground
Playground to implement list-scheduling in C++.

## Overview

The intial implementation is based on List-Scheduling for high-level synthesis (2x MUL / 1x ALU) functional units.
(see Branch initial)

The further progress is scheduling instructions towards individual functional units.
To speedup calculation, each instruction starts from the earliest possibile issue time.
If any previous instruction starts using the same functional units at a later time, we need to reschedule (starting from this instruction).

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

<img title="Example dependency graph" alt="Alt text" src="./images/dependecyGraph.png" style="background-color: white">

