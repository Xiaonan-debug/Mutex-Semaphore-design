/*
 *
 *  Producer-Consumer Lab
 *
 *  Copyright (c) 2022 Peter A. Dinda, Branden Ghena
 *
 */

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <semaphore.h>

#include "atomics.h"
#include "config.h"

//
// The interrupt_disable_save and interrupt_enable_restore functions
// emulate the interrupt control we have within an actual kernel
// using the analogous mechanisms in userspace, namely, signal
// masking
//

// disables interrupts, saving previous interrupt state
void interrupt_disable_save(int* oldstate) {
  sigset_t cur;
  sigemptyset(&cur);

  sigprocmask(SIG_BLOCK, NULL, &cur); // get current mask

  *oldstate = sigismember(&cur, INTERRUPT_SIGNAL); // stash "interrupt" state

  sigaddset(&cur, INTERRUPT_SIGNAL); // mask "interrupt"

  sigprocmask(SIG_BLOCK, &cur, NULL); // set new mask
}


// restores state of interrupts from argument
void interrupt_enable_restore(int oldstate) {
  if (oldstate == 0) {
    // need to unblock
    sigset_t cur;
    sigemptyset(&cur);

    sigprocmask(SIG_BLOCK, NULL, &cur); // get current mask

    sigdelset(&cur, INTERRUPT_SIGNAL); // unmask "interrupt"

    sigprocmask(SIG_BLOCK, &cur, NULL); // set new mask
  }

  // otherwise there is nothing to do - it should stay blocked
}

// To implement a lock, access to the critical section is the one shared resource.
int init(sem_t* sem, unsigned int size) {
  return sem_init(sem, 0, size);
}

// Lock / Down, decreases the value and waits if negative.
// sem_wait would return without succeeding.
// for example, occurrence of signal or interrupts
int wait(sem_t* sem) {
  while (sem_wait(sem) != 0) {}
  return 0;
}

// Try to Lock / Down, decreases the value and waits if negative.
// if could not lock, returns an error
int try_wait(sem_t* sem) {
  return sem_trywait(sem);
}

// Unlock / Up, increases the value and wakes one waiting thread.
int post(sem_t* sem) {
  return sem_post(sem);
}