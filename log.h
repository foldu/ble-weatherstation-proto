#ifndef WEATHERSTATION_LOG_H
#define WEATHERSTATION_LOG_H

/**
 * @file
 * Logging functions.
 */

#ifdef LOG_UART
#include <stdio.h>
/// Print log to UART, syntax like printf but with a mandatory string literal
/// as the first argument.
#define LOG(fmt, ...) (printf(fmt "\n", ##__VA_ARGS__))
#endif

#ifdef LOG_RBUF
#error Log ring buffer currently unimplemented
#endif

#endif
