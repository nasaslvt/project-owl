#!/bin/bash

config-pin P8_15 pruin
config-pin P8_27 pruin

make TARGET=swivel.pru0 PRU_DIR=/sys/class/remoteproc/remoteproc0
make TARGET=rotate.pru1 PRU_DIR=/sys/class/remoteproc/remoteproc1
