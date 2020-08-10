#ifndef WS_UTIL_H
#define WS_UTIL_H

/**
 * @file
 * Various utility functions.
 */

/**
 * Calculate the amount of elements in a array.
 * @warning Do not call this on decayed arrays.
 */
#define STATIC_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

#endif
