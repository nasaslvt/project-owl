# project-owl

## Configuration
### BeagleBone Black coldstart
Download AM3358 Debian 10.3 image from https://beagleboard.org/latest-images. Unzip using 7zip. Flash onto SD card using Rufus.
Insert SD card into BeagleBone. Hold down BOOT pushbutton (the one near the SD card) and insert power to the board. 

Configure passwords and users
```bash
su - 
passwd
adduser slvt
usermod -aG sudo slvt
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

## Operation
### Using rtl_fm
Reference guide: http://kmkeen.com/rtl-demod-guide/
To listen to FM radio: `rtl_fm -M wbfm -f 90.7M | sox -r 32k -t raw -e s -b 16 -c 1 -V1 - test.wav`
To use rtl_fm and Dire Wolf: `rtl_fm  -f 145.70M - | direwolf -c receive.conf` or run `receive.sh`.
We are using **145.70 MHz** as our test frequency, although APRS for North America is 144.39M. 

## Debugging
The user running Dire Wolf must be a member of the `audio` group.

