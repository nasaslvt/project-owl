/*
Copyright (c) 2013 Adafruit
Author: Justin Cooper

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "c_adc.h"
#include "common.h"

    char adc_prefix_dir[49];

int adc_initialized = 0;

BBIO_err initialize_adc(void)
{
    char test_path[149];
    FILE *fh;
    BBIO_err err;

    if (adc_initialized) {
        return BBIO_OK;
    }

    err = load_device_tree("BB-ADC");
    if (err == BBIO_OK) {
        strncat(adc_prefix_dir, "/sys/bus/iio/devices/iio:device0/in_voltage", sizeof(adc_prefix_dir));
        snprintf(test_path, sizeof(test_path), "%s%d_raw", adc_prefix_dir, 1);
        sleep(1);
        fh = fopen(test_path, "r");

        if (!fh) {
            return BBIO_SYSFS;
        }
        fclose(fh);

        adc_initialized = 1;
        return BBIO_OK;
    }

    return BBIO_GEN;
}

BBIO_err read_value(unsigned int ain, float *value)
{
    FILE * fh;
    char ain_path[149];
    snprintf(ain_path, sizeof(ain_path), "%s%d_raw", adc_prefix_dir, ain);

    int err, try_count=0;
    int read_successful;
    
    read_successful = 0;

    // Workaround to AIN bug where reading from more than one AIN would cause access failures
    while (!read_successful && try_count < 3)
    {
        fh = fopen(ain_path, "r");

        // Likely a bad path to the ocp device driver 
        if (!fh) {
            return BBIO_SYSFS;
        }

        fseek(fh, 0, SEEK_SET);
        err = fscanf(fh, "%f", value);

        if (err != EOF) read_successful = 1;
        fclose(fh);

        try_count++;
    }

    if (read_successful) return BBIO_OK;

    // Fall through and fail
    return BBIO_GEN;
}

BBIO_err adc_setup(void)
{
    return initialize_adc();
}

BBIO_err adc_cleanup(void)
{
    return unload_device_tree("BB-ADC");
}
