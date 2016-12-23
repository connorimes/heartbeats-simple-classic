// force assertions
#undef NDEBUG
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "heartbeats-simple-classic.h"

static const uint64_t WINDOW_SIZE = 10;
static const char* LOGFILE = "heartbeat.log";
static const uint64_t ITERATIONS = 20;
static const uint64_t WORK = 1;
static const uint64_t ACCURACY = 1;

void test_hbsc() {
  hbsc_ctx hb;
  assert(hbsc_init(&hb, WINDOW_SIZE, LOGFILE) == 0);
  assert(hbsc_get_hb(&hb));
  uint64_t i;
  for (i = 0; i < ITERATIONS; i++) {
    hbsc(&hb, i, WORK);
  }
  assert(hbsc_finish(&hb) == 0);
}

void test_hbsc_acc() {
  hbsc_acc_ctx hb;
  assert(hbsc_acc_init(&hb, WINDOW_SIZE, LOGFILE) == 0);
  assert(hbsc_acc_get_hb(&hb));
  uint64_t i;
  for (i = 0; i < ITERATIONS; i++) {
    hbsc_acc(&hb, i, WORK, ACCURACY);
  }
  assert(hbsc_acc_finish(&hb) == 0);
}

void test_hbsc_pow() {
  hbsc_pow_ctx hb;
  assert(hbsc_pow_init(&hb, WINDOW_SIZE, LOGFILE) == 0);
  assert(hbsc_pow_get_hb(&hb));
  uint64_t i;
  for (i = 0; i < ITERATIONS; i++) {
    hbsc_pow(&hb, i, WORK);
  }
  assert(hbsc_pow_finish(&hb) == 0);
}

void test_hbsc_acc_pow() {
  hbsc_acc_pow_ctx hb;
  assert(hbsc_acc_pow_init(&hb, WINDOW_SIZE, LOGFILE) == 0);
  assert(hbsc_acc_pow_get_hb(&hb));
  uint64_t i;
  for (i = 0; i < ITERATIONS; i++) {
    hbsc_acc_pow(&hb, i, WORK, ACCURACY);
  }
  assert(hbsc_acc_pow_finish(&hb) == 0);
}

int main(void) {
  test_hbsc();
  test_hbsc_acc();
  test_hbsc_pow();
  test_hbsc_acc_pow();
  return 0;
}
