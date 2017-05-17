/**
 * Implementation of heartbeats-simple-classic.h.
 *
 * @author Connor Imes
 * @date 2016-06-16
 */
#include <energymon-default.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "heartbeats-simple-classic.h"

static uint64_t get_time_ns(void) {
  static const uint64_t ONE_BILLION = 1000000000;
  return omp_get_wtime() * ONE_BILLION;
}

static int hbsc_init_fail(int log_fd, void* window_buffer) {
  int err_save = errno;
  if (log_fd > 0) {
    close(log_fd);
  }
  free(window_buffer);
  errno = err_save;
  return -1;
}

/**
 * Init
 */
#if defined(HEARTBEAT_MODE_ACC)
int hbsc_acc_init(hbsc_acc_ctx* hb, uint64_t window_size, const char* log_name) {
  size_t record_size = sizeof(heartbeat_acc_record);
#elif defined(HEARTBEAT_MODE_POW)
int hbsc_pow_init(hbsc_pow_ctx* hb, uint64_t window_size, const char* log_name) {
  size_t record_size = sizeof(heartbeat_pow_record);
#elif defined(HEARTBEAT_MODE_ACC_POW)
int hbsc_acc_pow_init(hbsc_acc_pow_ctx* hb, uint64_t window_size, const char* log_name) {
  size_t record_size = sizeof(heartbeat_acc_pow_record);
#else
int hbsc_init(hbsc_ctx* hb, uint64_t window_size, const char* log_name) {
  size_t record_size = sizeof(heartbeat_record);
#endif
  if (hb == NULL || window_size == 0) {
    errno = EINVAL;
    return -1;
  }

  // allocate window buffer
  hb->window_buffer = malloc(window_size * record_size);
  if (hb->window_buffer == NULL) {
    return -1;
  }

  if (log_name == NULL) {
    hb->log_fd = 0;
  } else {
    // open log file
    hb->log_fd = open(log_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (hb->log_fd < 0) {
      return hbsc_init_fail(hb->log_fd, hb->window_buffer);
    }
    // write log file header (and verify that writing works)
#if defined(HEARTBEAT_MODE_ACC)
    if (hb_acc_log_header(hb->log_fd)) {
#elif defined(HEARTBEAT_MODE_POW)
    if (hb_pow_log_header(hb->log_fd)) {
#elif defined(HEARTBEAT_MODE_ACC_POW)
    if (hb_acc_pow_log_header(hb->log_fd)) {
#else
    if (hb_log_header(hb->log_fd)) {
#endif
      return hbsc_init_fail(hb->log_fd, hb->window_buffer);
    }
  }

  hb->start_time = 0;

#if defined(HEARTBEAT_MODE_POW) || defined(HEARTBEAT_MODE_ACC_POW)
  // init energymon
  if (energymon_get_default(&hb->em) || hb->em.finit == NULL || hb->em.fread == NULL || hb->em.finit(&hb->em)) {
    return hbsc_init_fail(hb->log_fd, hb->window_buffer);
  }
  hb->start_energy = 0;
#endif

  // per the heartbeats API, these should never fail since we verified parameters
#if defined(HEARTBEAT_MODE_ACC)
  heartbeat_acc_init(&hb->hb, window_size, hb->window_buffer, hb->log_fd, NULL);
#elif defined(HEARTBEAT_MODE_POW)
  heartbeat_pow_init(&hb->hb, window_size, hb->window_buffer, hb->log_fd, NULL);
#elif defined(HEARTBEAT_MODE_ACC_POW)
  heartbeat_acc_pow_init(&hb->hb, window_size, hb->window_buffer, hb->log_fd, NULL);
#else
  heartbeat_init(&hb->hb, window_size, hb->window_buffer, hb->log_fd, NULL);
#endif

  return 0;
}

/**
 * Heartbeat
 */
#if defined(HEARTBEAT_MODE_ACC)
int hbsc_acc(hbsc_acc_ctx* hb, uint64_t user_tag, uint64_t work, uint64_t accuracy) {
#elif defined(HEARTBEAT_MODE_POW)
int hbsc_pow(hbsc_pow_ctx* hb, uint64_t user_tag, uint64_t work) {
#elif defined(HEARTBEAT_MODE_ACC_POW)
int hbsc_acc_pow(hbsc_acc_pow_ctx* hb, uint64_t user_tag, uint64_t work, uint64_t accuracy) {
#else
int hbsc(hbsc_ctx* hb, uint64_t user_tag, uint64_t work) {
#endif
  if (hb == NULL) {
    errno = EINVAL;
    return -1;
  }
  // start values are last hb's end values
#if defined(HEARTBEAT_MODE_POW) || defined(HEARTBEAT_MODE_ACC_POW)
  const uint64_t start_energy = hb->start_energy;
  errno = 0;
  hb->start_energy = hb->em.fread(&hb->em);
  if (hb->start_energy == 0 && errno) {
    return -1;
  }
#endif
  const uint64_t start_time = hb->start_time;
  hb->start_time = get_time_ns();
  // the first request doesn't actually issue a heartbeat (not enough data yet)
  if (start_time > 0) {
#if defined(HEARTBEAT_MODE_ACC)
    heartbeat_acc(&hb->hb, user_tag, work, start_time, hb->start_time, accuracy);
#elif defined(HEARTBEAT_MODE_POW)
    heartbeat_pow(&hb->hb, user_tag, work, start_time, hb->start_time, start_energy, hb->start_energy);
#elif defined(HEARTBEAT_MODE_ACC_POW)
    heartbeat_acc_pow(&hb->hb, user_tag, work, start_time, hb->start_time, accuracy, start_energy, hb->start_energy);
#else
    heartbeat(&hb->hb, user_tag, work, start_time, hb->start_time);
#endif
  }
  return 0;
}

/**
 * Finish
 */
#if defined(HEARTBEAT_MODE_ACC)
int hbsc_acc_finish(hbsc_acc_ctx* hb) {
#elif defined(HEARTBEAT_MODE_POW)
int hbsc_pow_finish(hbsc_pow_ctx* hb) {
#elif defined(HEARTBEAT_MODE_ACC_POW)
int hbsc_acc_pow_finish(hbsc_acc_pow_ctx* hb) {
#else
int hbsc_finish(hbsc_ctx* hb) {
#endif
  if (hb == NULL) {
    errno = EINVAL;
    return -1;
  }
  int ret = 0;
  // destroy energymon
#if defined(HEARTBEAT_MODE_POW) || defined(HEARTBEAT_MODE_ACC_POW)
  if (hb->em.ffinish != NULL) {
    ret |= hb->em.ffinish(&hb->em);
  }
#endif
  if (hb->log_fd > 0) {
    // flush remaining log entries
#if defined(HEARTBEAT_MODE_ACC)
    ret |= hb_acc_log_window_buffer(&hb->hb, hb->log_fd);
#elif defined(HEARTBEAT_MODE_POW)
    ret |= hb_pow_log_window_buffer(&hb->hb, hb->log_fd);
#elif defined(HEARTBEAT_MODE_ACC_POW)
    ret |= hb_acc_pow_log_window_buffer(&hb->hb, hb->log_fd);
#else
    ret |= hb_log_window_buffer(&hb->hb, hb->log_fd);
#endif
    // close log file
    ret |= close(hb->log_fd);
  }
  // cleanup memory
  free(hb->window_buffer);
  return ret;
}

/**
 * Get underlying heartbeat
 */
#if defined(HEARTBEAT_MODE_ACC)
heartbeat_acc_context* hbsc_acc_get_hb(hbsc_acc_ctx* hb) {
#elif defined(HEARTBEAT_MODE_POW)
heartbeat_pow_context* hbsc_pow_get_hb(hbsc_pow_ctx* hb) {
#elif defined(HEARTBEAT_MODE_ACC_POW)
heartbeat_acc_pow_context* hbsc_acc_pow_get_hb(hbsc_acc_pow_ctx* hb) {
#else
heartbeat_context* hbsc_get_hb(hbsc_ctx* hb) {
#endif
  if (hb == NULL) {
    errno = EINVAL;
    return NULL;
  }
  return &hb->hb;
}
