/**
 * @file driver_status.h
 * @brief Common status codes for register-level drivers.
 */

#ifndef DRIVER_STATUS_H
#define DRIVER_STATUS_H

typedef enum {
    DRIVER_STATUS_OK = 0,
    DRIVER_STATUS_INVALID_ARGUMENT,
    DRIVER_STATUS_TIMEOUT,
    DRIVER_STATUS_BUSY,
    DRIVER_STATUS_IO_ERROR,
    DRIVER_STATUS_NOT_READY,
    DRIVER_STATUS_OUT_OF_RANGE
} DriverStatus;

#endif /* DRIVER_STATUS_H */
