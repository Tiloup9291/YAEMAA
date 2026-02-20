# YAEMAA
## YAEMAA - Yet Another Ethercat Master And Application. Application tool to control ethercat slaves.
### Description
This is the first version of my application tool to control ethercat slaves. Right now it is fixed to manage an AMSAMOTION EC1A-IO16R.
This tool is a CLI tool and also a service. Both are accessible from the same binary.
### Prerequisites
- Linux 6.12.70 (RT) 64 bits
- Igh Etherlab ethercat-master 1.6.8
### Dependencies
- bc
- bison
- flex
- libssl-dev
- make
- autoconf
- automake
- libtool
- libbpf
- libsystemd-dev
### My setup
- Raspberry pi 4 B
- Debian 13 trixie raspberry pi OS Lite 64 bits
- AMSAMOTION EC1A-IO16R (Hardware version 0.1, Software version 0.1 (but ESI file tells revision number : 0x00010001. Label on device tells : v1.1)
- My setup right now have a full cycle time of 1 ms (including sleep) and a loop processing time of 40 us.
### Overview
You can find here the ESI file of this board, my kernel .config file, also my kernel boot option in cmdline.txt and of course the source files.
Native building on the running rpi 4b.
## Installation
### Kernel building command :
Building in the source directory.
If you want the same kernel configuration, please refers to my .config file here.
```
make oldconfig
make -j4 Image.gz modules dtbs
make modules_install
make headers_install
```
You can than copy (cp) the Image.gz file to your running board at /boot/firmware/kernel8.img
```
cp arch/arm64/boot/Image.gz /boot/firmware/kernel8.img
```
### Ethercat building command :
Building in the source directory.
```
./configure --sysconfdir=/etc --enable-genet --enable-kernel
make all modules
make modules_install install
```
### System configuration for managing ethercat :
1. Configure the ethercat master with config file at /etc/ethercat.conf.
Fill the MASTER0_DEVICE variable with your interface MAC address :
```
MASTER0_DEVICE="XX:XX:XX:XX:XX:XX"
```
And the NIC driver :
```
DEVICE_MODULES="genet"
```
2. Next, we must block linux network manager to try to take control of the NIC and create an interface.
Add the next lines in /etc/NetworkManager/NetworkManager.conf
```
[keyfile]
unmanaged-devices=interfaces-name:eth0
```
3. Next, we must block linux kernel to try to load the NIC module driver of your board.
Create a file in /etc/modprobe.d/blacklist-bcmgenet.conf and add following line
```
blacklist genet
```
4. You can reboot for change to take effect
```
reboot
```
5. Master useful commands :
To start/stop master
```
ethercatctl start
ethercatctl stop
```
To interrogate master :
```
ethercat master
ethercat slaves -v
ethercat domain
ethercat pdos
ethercat cstruct
ethercat xml
```
### Setup this application tool service :
copy ethercat_io.service of this project to /etc/systemd/system.
than reboot systemctl service to take new service :
```
systemctl daemon-reload
```
### Building this tool/service :
Building in source directory.
```
gcc -O2 -Wall *.c -o ethercat_ioctl -lethercat -lsystemd
install -m 755 ethercat_ioctl /bin
```
### This tool/service commands :
To start :
```
ethercat_ioctl start
```
To stop :
```
ethercat_ioctl stop
```
To view service status :
```
ethercat_ioctl status
```
to view full cycle time and loop processing time :
```
ethercat_ioctl live
```
To read input (ethercat_ioctl read [pdo] [subindex] [input position (0-15)]) :
```
ethercat_ioctl read 6000 0 3
```
to write output (ethercat_ioctl write [pdo] [subindex] [output position (0-15)] [value (1 = on, 0 = off)] :
```
ethercat_ioctl write 7000 0 5 1
```
## TODO list
1. Dynamically configure PDO/SDO from ESI (xml) device file;
2. Dynamically update available PDO/SDO/subindex/position/value of command read and write;
3. Protection for overflow of buffer, socket, array, datatype;
4. Closing faster;
