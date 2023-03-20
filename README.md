CS343 OS Project


# Producer-Consumer Lab
Copyright (c) 2022 Peter Dinda, Branden Ghena

The purpose of this lab is to introduce the student to the challenges of
designing and implementing a high performance concurrent data structure. Both
thread and interrupt concurrency are included.

The specific data structure is a producer-consumer queue implemented as a
classic ring-buffer. The driver includes control over the number of producer
and consumer threads, as well as whether interrupt handlers can be producers or
consumers. Rings are created with possible limits on the number of producers
and consumers, which makes it possible to focus on
single-producer-single-consumer, single-producer-multiple-consumer,
multiple-producer-single-consumer, and multiple-producer-multiple-producer
models.

Test your implementation by running `./harness`. Run `harness -h` to see the
available options.

## Files

Both implementation directories contain copies of the exact same starter code.
You will implement two different solutions to the problem, once using a mutex
that you build yourself out of atomic primitives and once with semaphores
provided by the `pthread` library.

 * `atomics.[ch]` - wrappers for hardware atomic operations
 * `ring.h`       - required ring interface for producer-consumer problem
 * `ring.c`       - implementation of ring *without* concurrency control
 * `harness.c`    - test harness 
 * `config.h`     - configuration, debug macros
 * `Makefile`     - build rules

Be aware that we will overwrite `harness.c`, `config.h`, and `Makefile` with
their default versions before running your code. We will also disable debug
prints in the `config.h` file (`#define DEBUG_OUTPUT 0`). Make sure that all of
your modifications are in `atomics.[ch]` or `ring.[ch]` and that the code runs
with the original `harness.c` implementation.

