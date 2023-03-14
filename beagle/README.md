# BeagleBone Black Wireless

The BeagleBone Black Wireless (BBB) is running the [Debian 10.3 2020-04-06 4GB eMMC IoT firmware](https://debian.beagleboard.org/images/bone-eMMC-flasher-debian-10.3-iot-armhf-2020-04-06-4gb.img.xz). This firmware is a Debian Buster IoT (without graphical desktop) image specifically designed for flashing the on-board eMMC memory of the BeagleBone via a microSD card.

The BBB is hosting a wireless access point with the SSID "owl". The network has a DHCP range of 192.168.8.50 to 192.168.8.150 and a listen address of 192.168.8.1. The ESP32-Cam is configured to connect to this network on startup with a static ip address of 192.168.8.10.

## Configuration

#### Flashing Firmware

* Use the Rufus software to flash the [Debian 10.3 2020-04-06 4GB eMMC IoT Flasher image](https://debian.beagleboard.org/images/bone-eMMC-flasher-debian-10.3-iot-armhf-2020-04-06-4gb.img.xz) onto the SD card. 
* Insert the SD card into the BeagleBone and hold down the "User Boot" button (located near the SD card slot). 
* Connect the power source (USB or 5V adaptor) while holding down the "User Boot" button.
* Release the button once the bank of 4 LED's light up for a few seconds.
* The flashing process will take approximately 30-45 minutes. 
* Once the flashing is complete, the bank of 4 LED's to the right of the Ethernet will turn off. 
* Power down the BeagleBone Black, remove the SD card, and power up the BeagleBone Black. 

* To connect to the BeagleBone via USB, use the following command:
```bash
ssh debian@192.168.7.1 or ssh debian@192.168.6.1
```
* When prompted, enter the password `temppwd`.

* Configure users and passwords
```bash
sudo su
passwd
adduser slvt
usermod -aG sudo slvt
```

* Setup Wireless Access Point
```bash
sudo sed -i -e 's:USE_PERSONAL_SSID="BeagleBone":USE_PERSONAL_SSID="owl":g' /etc/default/bb-wl18xx
sudo sed -i -e 's:USE_PERSONAL_PASSWORD="BeagleBone":USE_PERSONAL_PASSWORD="str1g1f0rm35":g' /etc/default/bb-wl18xx
sudo sed -i -e 's:USE_APPENDED_SSID=yes:USE_APPENDED_SSID=no:g' /etc/default/bb-wl18xx
sudo reboot now
```

