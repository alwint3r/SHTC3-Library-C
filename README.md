Unofficial Sensirion's SHTC3 Temperature and Humidity Sensor Library In C
=========================================================================

This library is created to handle the operating mode and measurement commands without assuming the underlying I2C implementation of the microcontroller or microprocessor that is being used.

As the consequence, you will need to implement the I2C write and read, also the delay function according to the SDK or framework that you use.

## Usage

### I2C Write Implementation

This is an example of the implementation using Mbed OS

```c
shtc3_error_t i2c_write(shtc3_i2c_config_t* config, const uint8_t* data, size_t length)
{
    int result = i2c.write((config->i2c_address << 1), (const char*)data, length);

    if (result == 0)
    {
        return SHTC3_OK;
    }
    else
    {
        return SHTC3_ERROR;
    }
}
```

### I2C Read Implementation

This is an example of the implementation using Mbed OS


```c
shtc3_error_t i2c_read(shtc3_i2c_config_t* config, uint8_t* data, size_t length)
{
    int result = i2c.read((config->i2c_address << 1), (char*)data, length);

    if (result == 0)
    {
        return SHTC3_OK;
    }
    else
    {
        return SHTC3_ERROR;
    }
}
```

### Delay Function Implementation

This is an example of the implementation using Mbed OS

```c
void delay_fn(uint32_t milliseconds)
{
    thread_sleep_for(milliseconds);
}
```

### Initialization

Having these functions defined, you can then initialize the sensor library

```c
shtc3_t *sensor = shtc3_init(i2c_read, i2c_write, delay_fn, 0x70);
```

### Check Sensor Presence

You might want to check the presence of SHTC3 at address 0x70 before going further.

```c
shtc3_error_t err = shtc3_is_present(sensor);

if (err == SHTC3_ERROR)
{
    // do something if sensor is not present
}

// err equals to SHTC3_OK if sensor is present
```

### Sleep

You can save power by making the sensor enter the sleep mode

```c
shtc_error_t err = shtc3_sleep();
```

### Wake Up from Sleep

If your sensor is in sleep mode and you want to measure the temperature and humidity, you will need to wake it up first.

```c
shtc_error_t err = shtc3_wakeup();
```

### Measurement

SHTC3 supports the following combinations of measurement mode:

* Polling and low power mode
* Polling and normal mode
* Clock stretching enabled and low power mode
* Clock stretching enabled and normal node

Take a look at the `shtc3_measurement_mode_t` and `shtc3_measurement_power_mode_t` enums.

Here's an example of polling and low power measurement:

```c
shtc3_measurement_t measurement = {};

shtc3_error_t err = shtc3_measure(sensor, SHTC3_MEASUREMENT_POLL, SHTC3_MEASUREMENT_PWR_LOW, &measurement);

// measurement.temperature in celcius
// measurement.humidity in %
```

### Deinitialization

You may not need to do this as the sensor might live as long as the processor is running. Anyway, here's how it's done:

```c
shtc3_deinit(sensor);
```


### Credits

I wrote most of the code myself, except the crc8 calculation function which is taken directly from [this official example project for STM32](https://github.com/Sensirion/shtc3-stm-sample-project) written by Sensirion.