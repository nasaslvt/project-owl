# receive.sh
# Peter Costescu, SLVT, 10/29/2022
# Start rtl_fm outputting on stdout, and pipe to direwolf
rtl_fm -f 145.70M - | direwolf -c receive.conf