/**
 * Wrapper for heartbeats-simple that provides a more classic heartbeats style interface.
 *
 * @author Connor Imes
 * @date 2016-06-16
 */
#ifndef _HEARTBEATS_SIMPLE_CLASSIC_H_
#define _HEARTBEATS_SIMPLE_CLASSIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <energymon.h>
#include <heartbeats-simple.h>
#include <stdint.h>

typedef struct hbsc_ctx {
  heartbeat_context hb;
  heartbeat_record* window_buffer;
  int log_fd;
  uint64_t start_time;
} hbsc_ctx;

typedef struct hbsc_acc_ctx {
  heartbeat_acc_context hb;
  heartbeat_acc_record* window_buffer;
  int log_fd;
  uint64_t start_time;
} hbsc_acc_ctx;

typedef struct hbsc_pow_ctx {
  heartbeat_pow_context hb;
  heartbeat_pow_record* window_buffer;
  int log_fd;
  energymon em;
  uint64_t start_time;
  uint64_t start_energy;
} hbsc_pow_ctx;

typedef struct hbsc_acc_pow_ctx {
  heartbeat_acc_pow_context hb;
  heartbeat_acc_pow_record* window_buffer;
  int log_fd;
  energymon em;
  uint64_t start_time;
  uint64_t start_energy;
} hbsc_acc_pow_ctx;

/*
 * Initialize the heartbeat and any dependent data.
 *
 * @param hb
 *   Not NULL
 * @param window_size
 *   Must be > 0
 * @param log_name
 *   NULL if logging is not desired
 *
 * @return 0 on success, -1 on failure
 */
int hbsc_init(hbsc_ctx* hb, uint64_t window_size, const char* log_name);
int hbsc_acc_init(hbsc_acc_ctx* hb, uint64_t window_size, const char* log_name);
int hbsc_pow_init(hbsc_pow_ctx* hb, uint64_t window_size, const char* log_name);
int hbsc_acc_pow_init(hbsc_acc_pow_ctx* hb, uint64_t window_size, const char* log_name);

/*
 * Issue a heartbeat.
 *
 * @param hb
 *   Not NULL
 * @param user_tag
 * @param work
 * @param accuracy
 *
 * @return 0 on success, -1 on failure
 */
int hbsc(hbsc_ctx* hb, uint64_t user_tag, uint64_t work);
int hbsc_acc(hbsc_acc_ctx* hb, uint64_t user_tag, uint64_t work, uint64_t accuracy);
int hbsc_pow(hbsc_pow_ctx* hb, uint64_t user_tag, uint64_t work);
int hbsc_acc_pow(hbsc_acc_pow_ctx* hb, uint64_t user_tag, uint64_t work, uint64_t accuracy);

/*
 * Flush any log data and cleanup resources.
 *
 * @param hb
 *   Not NULL
 *
 * @return 0 on success, -1 on failure
 */
int hbsc_finish(hbsc_ctx* hb);
int hbsc_acc_finish(hbsc_acc_ctx* hb);
int hbsc_pow_finish(hbsc_pow_ctx* hb);
int hbsc_acc_pow_finish(hbsc_acc_pow_ctx* hb);

/*
 * Utility function to get the underlying heartbeat struct.
 *
 * @param hb
 *   Not NULL
 *
 * @return context on success, NULL if hb param is NULL
 */
heartbeat_context* hbsc_get_hb(hbsc_ctx* hb);
heartbeat_acc_context* hbsc_acc_get_hb(hbsc_acc_ctx* hb);
heartbeat_pow_context* hbsc_pow_get_hb(hbsc_pow_ctx* hb);
heartbeat_acc_pow_context* hbsc_acc_pow_get_hb(hbsc_acc_pow_ctx* hb);

#ifdef __cplusplus
}
#endif

#endif
