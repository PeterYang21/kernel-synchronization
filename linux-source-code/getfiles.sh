#!/bin/sh

echo "Fetching QEMU"
wget http://www.cs.rochester.edu/courses/256/fall2015/QDGL/qemu-install.tar.gz

echo "Fetching OpenWRT Hard Disk Image"
wget http://www.cs.rochester.edu/courses/256/fall2015/QDGL/openwrt-15.05-x86-generic-combined-ext4.img

echo "Unpacking QEMU Binaries"
gunzip --stdout qemu-install.tar.gz | tar -xvf -
