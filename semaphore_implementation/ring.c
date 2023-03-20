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
#include <semaphore.h>

#include "atomics.h"
#include "config.h"
#include "ring.h"


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
  init(&ring->mutex, 1);
  init(&ring->producer, 1);
  init(&ring->consumer, 0);

  DEBUG("ring %p created\n", ring);

  // WRITE ME!
  //
  // any synchronization state would be initialized here
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

  DEBUG("starting push of %p to %p\n", data, ring);
  wait(&ring->mutex);
  while (!can_push(ring)) {
    post(&ring->mutex);
    wait(&ring->producer);
    wait(&ring->mutex);
  }
  ring->elements[ring->head % ring->size] = data;
  ring->head++;
  post(&ring->consumer);
  post(&ring->mutex);
  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_push(ring_t* ring, void* data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT
  if (try_wait(&ring->mutex) != 0) {
    return -1;
  }
  if (!can_push(ring)) {
    post(&ring->mutex);
    return 1;
  }
  ring->elements[ring->head % ring->size] = data;
  ring->head++;
  post(&ring->consumer);
  post(&ring->mutex);
  return 0;
}


// Consumer Side

// note that this is NOT synchronized and so prone to races!
int ring_pull(ring_t* ring, void** data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT

  wait(&ring->mutex);
  while (!can_pull(ring)) {
    post(&ring->mutex);
    wait(&ring->consumer);
    wait(&ring->mutex);
  }
  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;
  post(&ring->producer);
  post(&ring->mutex);
  return 0;
}

// note that this is NOT synchronized and so prone to races!
int ring_try_pull(ring_t* ring, void** data) {
  // WRITE ME!
  //
  // THIS IS WRONG IN THE PRESENCE OF CONCURRENCY
  // YOU NEED TO FIX IT
  if (try_wait(&ring->mutex) != 0) {
    return -1;
  }
  if (!can_pull(ring)) {
    post(&ring->mutex);
    return 1;
  }
  *data = ring->elements[ring->tail % ring->size];
  ring->tail++;
  post(&ring->producer);
  post(&ring->mutex);
  return 0;
}
