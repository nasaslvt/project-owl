# Programmable Real-Time Unit

## Overview

The BeagleBone has both an ARM processor running Linux and two Programmable Real-Time Units (PRUs). The PRUs have 32-bit cores which run independently of the ARM processor, therefore they can be programmed to respond quickly to inputs and produce very precisely timed outputs. This is necessary for our application, which requires the use of two Parallax Feedback 360° High-Speed Servos. These servos are controlled by a 50 Hz PWM signal, and also have a return signal line from an internal Hall effect sensor system that provides digital angular position feedback at a frequency of 910 Hz. To decode this signal in software, the PRU is used to handle the return signal, reading the data and sending it back to the ARM processor. The ARM processor then uses this data to control the servo, with each PRU reading the feedback signal of an individual servo.

## Configuration

In order to enable the swivel servo feedback pin (P8_15) and rotation servo feedback pin (P8_27) to work with the PRU, the pins must be explicitly configured. To configure P8_15, the command `config-pin P8_15 pruin` must be used. To configure P8_27, the virtual cape for HDMI functionality must first be disabled by uncommenting the `disable_uboot_overlay_video=1` line in `/boot/uEnv.txt` and rebooting. Then the command `config-pin P8_27 pruin` can be used to configure the pin.

## Usage

```
sudo ./start.sh
```

## References
1. [PRU Cookbook](https://beagleboard.org/static/prucookbook/)
2. [Frequency Measurement with PRU BeagleBone CYCLE Register](https://github.com/SuperEjik/frequency-measurement-with-PRU-BeagleBone-CYCLE-register-)
3. [Programming the BeagleBone PRU-ICSS](https://www.glennklockwood.com/embedded/beaglebone-pru.html)
4. [AM335x and AMIC110 Sitara™ Processors Technical Reference Manual](https://www.ti.com/lit/ug/spruh73q/spruh73q.pdf)
5. [BeagleBone Black System Reference Manual 2020](https://kilobaser.com/wp-content/uploads/2021/02/BBB_SRM.pdf)
6. [PRU Debugger](https://github.com/poopgiggle/prudebug)