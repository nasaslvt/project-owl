# project-owl

## Repository Structure
- `beagle`: BeagleBone Black source code
- `conf`: Configuration files, especially for Dire Wolf
- `doc`: Documentation, including block diagrams and schematics
- `esp`: ESP32-CAM source code
- `kicad`: KiCad project files

## Documentation

Schematics were created with KiCad and follow the \<version>.\<subversion> naming convention.  
Block diagrams were created with DigiKey's online [SchemeIt tool](https://www.digikey.com/schemeit/project/project-owl-26cf1e4f4a01456586e33678977ee045).

## Configuration

#### Setup

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

