#include "shtc3.h"
#include <stdlib.h>
#include <string.h>

typedef struct __shtc3_raw_measurement_t
{
    uint8_t temperature_raw[2];
    uint8_t humidity_raw[2];
    uint8_t temperature_crc;
    uint8_t humidity_crc;
} shtc3_raw_measurement_t;

static shtc3_raw_measurement_t shtc3_create_raw_measurement(uint8_t* data)
{
    shtc3_raw_measurement_t measurement = {};
    memcpy(measurement.temperature_raw, data, 2);
    measurement.temperature_crc = data[2];
    memcpy(measurement.humidity_raw, data + 3, 2);
    measurement.humidity_crc = data[5];

    return measurement;
}

static shtc3_error_t shtc3_clock_stretched_measurement(shtc3_t *sensor, shtc3_measurement_power_mode_t power_mode, shtc3_raw_measurement_t *raw)
{
    if (sensor == NULL)
    {
        return SHTC3_ERROR;
    }

    uint8_t cmd[2] = {0};
    if (power_mode == SHTC3_MEASUREMENT_PWR_NORMAL)
    {
        cmd[0] = SHTC3_CMD_MEASURE_T_RH_N_STRETCH_H;
        cmd[1] = SHTC3_CMD_MEASURE_T_RH_N_STRETCH_L;
    }
    else if (power_mode == SHTC3_MEASUREMENT_PWR_LOW)
    {
        cmd[0] = SHTC3_CMD_MEASURE_T_RH_L_STRETCH_H;
        cmd[1] = SHTC3_CMD_MEASURE_T_RH_L_STRETCH_L;
    }
    else
    {
        return SHTC3_ERROR;
    }

    shtc3_error_t result =  sensor->i2c_write(&sensor->config, cmd, 2);
    if (result == SHTC3_ERROR)
    {
        return result;
    }

    uint8_t output[6] = {0};
    result = sensor->i2c_read(&sensor->config, output, 6);
    if (result == SHTC3_ERROR)
    {
        return result;
    }

    *raw = shtc3_create_raw_measurement(output);

    return SHTC3_OK;
}

static shtc3_error_t shtc3_polling_measurement(shtc3_t *sensor, shtc3_measurement_power_mode_t power_mode, shtc3_raw_measurement_t *raw)
{
    if (sensor == NULL)
    {
        return SHTC3_ERROR;
    }

    uint8_t cmd[2] = {0};
    if (power_mode == SHTC3_MEASUREMENT_PWR_NORMAL)
    {
        cmd[0] = SHTC3_CMD_MEASURE_T_RH_N_POLL_H;
        cmd[1] = SHTC3_CMD_MEASURE_T_RH_N_POLL_L;
    }
    else if (power_mode == SHTC3_MEASUREMENT_PWR_LOW)
    {
        cmd[0] = SHTC3_CMD_MEASURE_T_RH_L_POLL_H;
        cmd[1] = SHTC3_CMD_MEASURE_T_RH_L_POLL_L;
    }
    else
    {
        return SHTC3_ERROR;
    }

    shtc3_error_t result =  sensor->i2c_write(&sensor->config, cmd, 2);
    if (result == SHTC3_ERROR)
    {
        return result;
    }

    uint8_t max_wait_times = 20;
    while (max_wait_times--)
    {
        uint8_t temp = 0x0;
        shtc3_error_t err = sensor->i2c_read(&sensor->config, &temp, 1);
        if (err == SHTC3_OK)
        {
            break;
        }

        sensor->delay(1);
    }

    uint8_t output[6] = {0};
    result = sensor->i2c_read(&sensor->config, output, 6);
    if (result == SHTC3_ERROR)
    {
        return result;
    }

    *raw = shtc3_create_raw_measurement(output);

    return SHTC3_OK;
}

static float shtc3_calculate_temperature(uint8_t *raw)
{
    uint16_t raw_temperature = (raw[0] << 8 | raw[1]);

    return -45.0 + (175.0 * (raw_temperature / 65536.0));
}

static float shtc3_calculate_humidity(uint8_t *raw)
{
    uint16_t raw_humidity = (raw[0] << 8 | raw[1]);

    return 100.0 * (raw_humidity / 65536.0);
}

static shtc3_measurement_t shtc3_calculate_measurement(shtc3_raw_measurement_t *raw)
{
    shtc3_measurement_t measurement = {0};
    measurement.temperature = shtc3_calculate_temperature(raw->temperature_raw);
    measurement.humidity = shtc3_calculate_humidity(raw->humidity_raw);

    return measurement;
}

// taken from https://github.com/Sensirion/shtc3-stm-sample-project/blob/master/Source/shtc3.c
static uint8_t shtc3_crc8(uint8_t *data, size_t length)
{
    uint16_t polynomial = 0x131;
    uint8_t bit;        // bit mask
    uint8_t crc = 0xFF; // calculated checksum
    uint8_t index;      // byte counter

    // calculates 8-Bit checksum with given polynomial
    for (index = 0; index < length; index++)
    {
        crc ^= (data[index]);
        for (bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ polynomial;
            }
            else
            {
                crc = (crc << 1);
            }
        }
    }

    return crc;
}

static shtc3_error_t shtc3_crc8_check(uint8_t *data, size_t length, uint8_t crc)
{
    if (data == NULL)
    {
        return SHTC3_ERROR;
    }

    uint8_t computed_crc = shtc3_crc8(data, length);

    if (computed_crc != crc)
    {
        return SHTC3_ERROR;
    }

    return SHTC3_OK;
}

shtc3_t* shtc3_init(shtc3_i2c_read_t read_fn, shtc3_i2c_write_t write_fn, shtc3_delay_t delay_fn, uint8_t address)
{
    shtc3_i2c_config_t config = {
        .i2c_address = address,
    };

    shtc3_t *sensor = (shtc3_t *)malloc(sizeof(shtc3_t));
    if (sensor == NULL)
    {
        return NULL;
    }

    sensor->config = config;
    sensor->i2c_read = read_fn;
    sensor->i2c_write = write_fn;
    sensor->delay = delay_fn;

    return sensor;
}

shtc3_error_t shtc3_deinit(shtc3_t *sensor)
{
    if (sensor == NULL)
    {
        return SHTC3_ERROR;
    }

    free(sensor);

    return SHTC3_OK;
}

shtc3_error_t shtc3_is_present(shtc3_t *sensor)
{
    if (sensor == NULL)
    {
        return SHTC3_ERROR;
    }

    shtc3_error_t result = SHTC3_ERROR;

    uint8_t data[2] = {SHTC3_CMD_GET_ID_H, SHTC3_CMD_GET_ID_L};
    result = sensor->i2c_write(&sensor->config, (const uint8_t *)data, 2);
    if (result == SHTC3_ERROR)
    {
        return result;
    }

    uint8_t id_result[3] = {0};
    result = sensor->i2c_read(&sensor->config, id_result, 3);
    if (result == SHTC3_ERROR)
    {
        return result;
    }

    result = shtc3_crc8_check(id_result, 2, id_result[2]);

    return result;
}

shtc3_error_t shtc3_sleep(shtc3_t *sensor)
{
    if (sensor == NULL)
    {
        return SHTC3_ERROR;
    }

    shtc3_error_t result = SHTC3_ERROR;

    uint8_t data[2] = {SHTC3_CMD_SLEEP_H, SHTC3_CMD_SLEEP_L};
    result = sensor->i2c_write(&sensor->config, (const uint8_t *)data, 2);
    if (result == SHTC3_ERROR)
    {
        return result;
    }

    return SHTC3_OK;
}

shtc3_error_t shtc3_wakeup(shtc3_t *sensor)
{
    if (sensor == NULL)
    {
        return SHTC3_ERROR;
    }

    shtc3_error_t result = SHTC3_ERROR;

    uint8_t data[2] = {SHTC3_CMD_WAKEUP_H, SHTC3_CMD_WAKEUP_L};
    result = sensor->i2c_write(&sensor->config, (const uint8_t *)data, 2);
    if (result == SHTC3_ERROR)
    {
        return result;
    }

    return SHTC3_OK;
}


shtc3_error_t shtc3_measure(shtc3_t *sensor, shtc3_measurement_mode_t mode, shtc3_measurement_power_mode_t power_mode, shtc3_measurement_t *measurement)
{
    if (sensor == NULL || measurement == NULL)
    {
        return SHTC3_ERROR;
    }

    shtc3_raw_measurement_t raw = {};
    shtc3_error_t result = SHTC3_ERROR;

    if (mode == SHTC3_MEASUREMENT_STRETCHED)
    {
        result = shtc3_clock_stretched_measurement(sensor, power_mode, &raw);   
    }
    else if (mode == SHTC3_MEASUREMENT_POLL)
    {
        result = shtc3_polling_measurement(sensor, power_mode, &raw);
    }
    else
    {
        return SHTC3_ERROR;
    }

    if (result == SHTC3_ERROR)
    {
        return result;
    }

    if (shtc3_crc8_check(raw.temperature_raw, 2, raw.temperature_crc) == SHTC3_ERROR)
    {
        return SHTC3_ERROR;
    }

    if (shtc3_crc8_check(raw.humidity_raw, 2, raw.humidity_crc) == SHTC3_ERROR)
    {
        return SHTC3_ERROR;
    }

    shtc3_measurement_t computed_measurement = shtc3_calculate_measurement(&raw);

    *measurement = computed_measurement;

    return SHTC3_OK;
}