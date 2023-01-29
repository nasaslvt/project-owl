# project-owl

## Repository Structure
- `beagle`: BeagleBone Black source code
- `conf`: Configuration files, especially for Dire Wolf
- `doc`: Documentation, including block diagrams and schematics
- `esp`: ESP32-CAM source code
- `kicad`: KiCad project files

## Documentation

The BeagleBone Black Wireless (BBB) is running the [Debian 10.3 2020-04-06 4GB eMMC IoT firmware](https://debian.beagleboard.org/images/bone-eMMC-flasher-debian-10.3-iot-armhf-2020-04-06-4gb.img.xz). This firmware is a Debian Buster IoT (without graphical desktop) image specifically designed for flashing the on-board eMMC memory of the BeagleBone via a microSD card.

The BBB is hosting a wireless access point with the SSID "owl". The network has a DHCP range of 192.168.8.50 to 192.168.8.150 and a listen address of 192.168.8.1. The ESP32-Cam is configured to connect to this network on startup with a static ip address of 192.168.8.10.

Schematics were created with KiCad and follow the \<version>.\<subversion> naming convention.  
Block diagrams were created with DigiKey's online [SchemeIt tool](https://www.digikey.com/schemeit/project/project-owl-26cf1e4f4a01456586e33678977ee045).

## Configuration
### BeagleBone Black Wireless

#### Flashing Firmware

* Use the Rufus software to flash the [Debian 10.3 2020-04-06 4GB eMMC IoT Flasher image](https://debian.beagleboard.org/images/bone-eMMC-flasher-debian-10.3-iot-armhf-2020-04-06-4gb.img.xz) onto the SD card. 
* Insert the SD card into the BeagleBone and hold down the "User Boot" button (located near the SD card slot). 
* Connect the power source (USB or 5V adaptor) while holding down the "User Boot" button.
* Release the button once the bank of 4 LED's light up for a few seconds.
* The flashing process will take approximately 30-45 minutes. 
* Once the flashing is complete, the bank of 4 LED's to the right of the Ethernet will turn off. 
* Power down the BeagleBone Black, remove the SD card, and power up the BeagleBone Black. 

#### Setup

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

Install rtl_test, rtl_fm, etc.
```bash
apt-get install rtl-sdr sox
```

Build and install the latest direwolf (1.6 at time of this writing)
```bash
sudo apt-get install git gcc g++ make cmake libasound2-dev libudev-dev
cd ~
mkdir src && cd src
git clone https://www.github.com/wb2osz/direwolf
mkdir build && cd build
cmake ..
make -j4
sudo make install
make install-conf
exit
```

Copy `receive.conf` from this repository to `~/conf/receive.conf`

### ESP32-Camera

#### Flashing

todo

## Operation
### Using rtl_fm
[Reference guide](http://kmkeen.com/rtl-demod-guide/).
To listen to FM radio: `rtl_fm -M wbfm -f 90.7M | sox -r 32k -t raw -e s -b 16 -c 1 -V1 - test.wav`
To use rtl_fm and Dire Wolf: `rtl_fm  -f 145.70M - | direwolf -c receive.conf` or run `receive.sh`.
We are using **145.70 MHz** as our test frequency, although APRS for North America is 144.39M. 

## Debugging
The user running Dire Wolf must be a member of the `audio` group.

## Licensing
The KiCad footprint and model for the L101011MS02Q were provided by SnapEDA under the CC-BY-SA 4.0 license: https://www.snapeda.com/parts/L101011MS02Q/C&K/view-part/?ref=digikey 

