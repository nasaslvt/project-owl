# Adafruit Beaglebone I/O C API

The library was originally forked from the excellent MIT Licensed [Adafruit's BeagleBone IO Python Library](https://github.com/adafruit/adafruit-beaglebone-io-python) library written by Justin Cooper.

This version of the library consists of only the core components, excluding the C++ and Python components.

Created using 
```bash
unifdef -m -DNO_PYTHON -U__cplusplus *.h
unifdef -m -DNO_PYTHON -U__cplusplus *.c
```
