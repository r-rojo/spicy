#!/bin/sh

echo "Preparing FreeBSD environment"
sysctl hw.model hw.machine hw.ncpu
set -e
set -x

# Detection of whether we run in the build directory requires `/proc`.
echo "proc /proc procfs rw,noauto 0 0" >> /etc/fstab
mount /proc

env ASSUME_ALWAYS_YES=YES pkg bootstrap
pkg install -y bash git cmake flex bison python3 ninja llvm11 zeek base64 ccache

pyver=$(python3 -c 'import sys; print(f"py{sys.version_info[0]}{sys.version_info[1]}")')
pkg install -y "$pyver"-pip
pip install btest

# While the Zeek port does contain the Zeek CMake files, that package version
# is still not present in Cirrus CI images, so we patch the files in by hand.
# The branch we pick here roughly matches the version of zeek packaged in above
# port.
git clone --branch release/zeek-3.0 https://github.com/zeek/cmake /usr/local/share/zeek/cmake
