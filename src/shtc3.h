#ifndef SHTC3_LIBRARY_H
#define SHTC3_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#define SHTC3_DEFAULT_I2C_ADDR 0x70

#define SHTC3_CMD_GET_ID_H 0xEF
#define SHTC3_CMD_GET_ID_L 0xC8
#define SHTC3_CMD_SLEEP_H 0xB0
#define SHTC3_CMD_SLEEP_L 0x98
#define SHTC3_CMD_WAKEUP_H 0x35
#define SHTC3_CMD_WAKEUP_L 0x17
#define SHTC3_CMD_MEASURE_T_RH_N_POLL_H 0x78
#define SHTC3_CMD_MEASURE_T_RH_N_POLL_L 0x66
#define SHTC3_CMD_MEASURE_T_RH_N_STRETCH_H 0x7C
#define SHTC3_CMD_MEASURE_T_RH_N_STRETCH_L 0xA2
#define SHTC3_CMD_MEASURE_T_RH_L_POLL_H 0x60
#define SHTC3_CMD_MEASURE_T_RH_L_POLL_L 0x9C
#define SHTC3_CMD_MEASURE_T_RH_L_STRETCH_H 0x64
#define SHTC3_CMD_MEASURE_T_RH_L_STRETCH_L 0x58


typedef enum __shtc3_error_t
{
    SHTC3_OK,
    SHTC3_ERROR,
} shtc3_error_t;

typedef struct __shtc3_i2c_config_t
{
    uint8_t i2c_address;
} shtc3_i2c_config_t;

typedef shtc3_error_t (*shtc3_i2c_write_t)(shtc3_i2c_config_t*, const uint8_t*, size_t);

typedef shtc3_error_t (*shtc3_i2c_read_t)(shtc3_i2c_config_t*, uint8_t*, size_t);

typedef void (*shtc3_delay_t)(uint32_t);

typedef struct __shtc3_t
{
    shtc3_i2c_read_t i2c_read;
    shtc3_i2c_write_t i2c_write;
    shtc3_delay_t delay;
    shtc3_i2c_config_t config;
} shtc3_t;

typedef struct __shtc3_measurement_t
{
    float temperature;
    float humidity;
} shtc3_measurement_t;

typedef enum __shtc3_measurement_mode_t
{
    SHTC3_MEASUREMENT_STRETCHED,
    SHTC3_MEASUREMENT_POLL,
} shtc3_measurement_mode_t;

typedef enum __shtc3_measurement_power_mode_t
{
    SHTC3_MEASUREMENT_PWR_NORMAL,
    SHTC3_MEASUREMENT_PWR_LOW,
} shtc3_measurement_power_mode_t;

shtc3_t* shtc3_init(shtc3_i2c_read_t read_fn, shtc3_i2c_write_t write_fn, shtc3_delay_t delay_fn, uint8_t address);
shtc3_error_t shtc3_deinit(shtc3_t* sensor);
shtc3_error_t shtc3_is_present(shtc3_t* sensor);
shtc3_error_t shtc3_sleep(shtc3_t* sensor);
shtc3_error_t shtc3_wakeup(shtc3_t* sensor);
shtc3_error_t shtc3_measure(shtc3_t* sensor, shtc3_measurement_mode_t mode, shtc3_measurement_power_mode_t power_mode, shtc3_measurement_t* measurement);

#ifdef __cplusplus
}
#endif

#endif