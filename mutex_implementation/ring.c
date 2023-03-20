/*
 *
 *  Producer-Consumer Lab
 *
 *  Copyright (c) 2022 Peter A. Dinda, Branden Ghena
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "atomics.h"
#include "config.h"
#include "ring.h"

void cond_wait(bool* condvar, lock_t* lock){
  *condvar = false;
  mutex_release(lock);
  while(!*condvar);
  mutex_acquire(lock);
}

void cond_signal(bool* condvar){
  *condvar = true;
}


ring_t* ring_create(uint32_t size,
                    uint32_t producer_limit,
                    int      producer_interrupts,
                    uint32_t consumer_limit,
                    int      consumer_interrupts) {
  // the handout version does not observe the limits or interrupts

  uint64_t numbytes = sizeof(ring_t) + (size * sizeof(void*));
  ring_t* ring      = (ring_t*)malloc(numbytes);

  if (!ring) {
    ERROR("Cannot allocate\n");
    return 0;
  }

  memset(ring, 0, numbytes);

  DEBUG("allocation is at %p, data at %p\n", ring, ring->elements);

  ring->size = size;
  ring->head = 0;
  ring->tail = 0;

  // WRITE ME!
  //
  // any synchronization state would be initialized here
  ring->ready_push = true;
  ring->ready_pull = true;
  ring->mutex = malloc(sizeof(lock_t));
  mutex_init(ring->mutex);

  DEBUG("ring %p created\n", ring);

  return ring;
}

void ring_destroy(ring_t* ring) {
  DEBUG("destroy %p\n", ring);
  // do we need to wait for requests to finish?
  free(ring);
}


// Helper Functions

// note that this is NOT synchronized and so prone to races
static bool can_push(ring_t* ring) {
  return (ring->head - ring->tail) < ring->size;
}

// note that this is NOT synchronized and so prone to races!
static bool can_pull(ring_t* ring) {
  return ring->tail < ring->head;
}


// Producer Side

// note that this is NOT synchronized and so prone to races!


int ring_push(ring_t* ring, void* data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT
  int olddate;
  interrupt_disable_save(&olddate);
  mutex_acquire(ring->mutex);

  while(!can_push(ring)){
    ring->ready_push = false;
    mutex_release(ring->mutex);
    interrupt_enable_restore(olddate);
    while(!ring->ready_push);
    interrupt_disable_save(&olddate);
    mutex_acquire(ring->mutex);
  }
  //DEBUG("starting push of %p to %p\n", data, ring);
  //DEBUG("%ld push %ld\n", ring->head, ring->size);

  ring->elements[ring->head % ring->size] = data;
  ring->head++;
  //DEBUG("push done\n");
  cond_signal(&ring->ready_pull);

  mutex_release(ring->mutex);
  interrupt_enable_restore(olddate);

  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_push(ring_t* ring, void* data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT
  int result = try_lock(ring->mutex);
  if(result == -1){
    return -1;
  }
  
  if (!can_push(ring)) {
    mutex_release(ring->mutex);
    return 1;
  }

  ring->elements[ring->head % ring->size] = data;
  ring->head++;
  cond_signal(&ring->ready_pull);
  mutex_release(ring->mutex);

  return 0;
}


// Consumer Side

// note that this is NOT synchronized and so prone to races!

int ring_pull(ring_t* ring, void** data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT
  int olddate;
  interrupt_disable_save(&olddate);
  mutex_acquire(ring->mutex);

  while(!can_pull(ring)){
    ring->ready_pull = false;
    mutex_release(ring->mutex);
    interrupt_enable_restore(olddate);
    while(!ring->ready_pull);
    interrupt_disable_save(&olddate);
    mutex_acquire(ring->mutex);
  }
  //DEBUG("starting pull from %p\n", ring);
  //DEBUG("%ld pull %ld\n", ring->tail, ring->size);

  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;
  //DEBUG("pulled %p\n", *data);
  cond_signal(&ring->ready_push);

  mutex_release(ring->mutex);
  interrupt_enable_restore(olddate);

  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_pull(ring_t* ring, void** data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT
  int result = try_lock(ring->mutex);
  if(result == -1){
    return -1;
  }
  
  if (!can_pull(ring)) {
    mutex_release(ring->mutex);
    return 1;
  }

  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;
  cond_signal(&ring->ready_push);
  mutex_release(ring->mutex);

  return 0;
}



