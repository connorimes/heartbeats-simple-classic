/**
 * Implementation of heartbeats-simple-classic.h.
 *
 * @author Connor Imes
 * @date 2016-06-16
 */
#include <energymon-default.h>
#include <errno.h>
#include <fcntl.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "heartbeats-simple-classic.h"

static uint64_t get_time_ns() {
  static const uint64_t ONE_BILLION = 1000000000;
  return omp_get_wtime() * ONE_BILLION;
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
  int err_save = 0;

  // allocate window buffer
  hb->window_buffer = malloc(window_size * record_size);
  if (hb->window_buffer == NULL) {
    return -1;
  }

  hb->log_fd = 0;
  if (log_name != NULL) {
    // open log file
    hb->log_fd = open(log_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (hb->log_fd < 0) {
      free(hb->window_buffer);
      return -1;
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
      err_save = errno;
      close(hb->log_fd);
      free(hb->window_buffer);
      errno = err_save;
      return -1;
    }
  }

  hb->start_time = 0;

  // init energymon
#if defined(HEARTBEAT_MODE_POW) || defined(HEARTBEAT_MODE_ACC_POW)
  if (energymon_get_default(&hb->em) || hb->em.finit(&hb->em)) {
    // safer to force NULL values in case users to continue with it after failure
    memset(&hb->em, 0, sizeof(energymon));
    return -1;
  }
  hb->start_energy = 0;
#endif

  // shouldn't fail unless we did something stupid
#if defined(HEARTBEAT_MODE_ACC)
    if (heartbeat_acc_init(&hb->hb, window_size, hb->window_buffer, hb->log_fd, NULL)) {
#elif defined(HEARTBEAT_MODE_POW)
    if (heartbeat_pow_init(&hb->hb, window_size, hb->window_buffer, hb->log_fd, NULL)) {
#elif defined(HEARTBEAT_MODE_ACC_POW)
    if (heartbeat_acc_pow_init(&hb->hb, window_size, hb->window_buffer, hb->log_fd, NULL)) {
#else
    if (heartbeat_init(&hb->hb, window_size, hb->window_buffer, hb->log_fd, NULL)) {
#endif
    err_save = errno;
    close(hb->log_fd);
    free(hb->window_buffer);
    errno = err_save;
    return -1;
  }

  return 0;
}

/**
 * Heartbeat
 */
#if defined(HEARTBEAT_MODE_ACC)
void hbsc_acc(hbsc_acc_ctx* hb, uint64_t user_tag, uint64_t work, uint64_t accuracy) {
#elif defined(HEARTBEAT_MODE_POW)
void hbsc_pow(hbsc_pow_ctx* hb, uint64_t user_tag, uint64_t work) {
#elif defined(HEARTBEAT_MODE_ACC_POW)
void hbsc_acc_pow(hbsc_acc_pow_ctx* hb, uint64_t user_tag, uint64_t work, uint64_t accuracy) {
#else
void hbsc(hbsc_ctx* hb, uint64_t user_tag, uint64_t work) {
#endif
  if (hb == NULL || hb->window_buffer == NULL) {
    errno = EINVAL;
    return;
  }
  // start values are last hb's end values
  const uint64_t start_time = hb->start_time;
  const uint64_t end_time = get_time_ns();
  hb->start_time = end_time;
#if defined(HEARTBEAT_MODE_POW) || defined(HEARTBEAT_MODE_ACC_POW)
  const uint64_t start_energy = hb->start_energy;
  const uint64_t end_energy = hb->em.fread(&hb->em);
  hb->start_energy = end_energy;
#endif
  // the first request doesn't actually issue a heartbeat (not enough data yet)
  if (start_time > 0) {
#if defined(HEARTBEAT_MODE_ACC)
    heartbeat_acc(&hb->hb, user_tag, work, start_time, end_time, accuracy);
#elif defined(HEARTBEAT_MODE_POW)
    heartbeat_pow(&hb->hb, user_tag, work, start_time, end_time, start_energy, end_energy);
#elif defined(HEARTBEAT_MODE_ACC_POW)
    heartbeat_acc_pow(&hb->hb, user_tag, work, start_time, end_time, accuracy, start_energy, end_energy);
#else
    heartbeat(&hb->hb, user_tag, work, start_time, end_time);
#endif
  }
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
  hb->window_buffer = NULL;
  // destroy energymon
#if defined(HEARTBEAT_MODE_POW) || defined(HEARTBEAT_MODE_ACC_POW)
  ret |= hb->em.ffinish(&hb->em);
  memset(&hb->em, 0, sizeof(energymon));
#endif
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
