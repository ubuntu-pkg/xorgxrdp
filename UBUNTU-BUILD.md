# Building Ubuntu package

Tested on Ubuntu 16.04

* Install matching version of xrdp (e.g. from https://github.com/ubuntu-pkg/xrdp/releases)
* Create an empty working directory
 ```
mkdir ~/xorgxrdp-build
cd ~/xorgxrdp-build
 ```

* Install matching version of xrdp (e.g. from https://github.com/ubuntu-pkg/xrdp/releases)

* Install dependencies
 ```
apt install git devscripts equivs gdebi-core
git clone -b ubuntu-devel https://github.com/ubuntu-xrdp/xorgxrdp.git
cd xorgxrdp
mk-build-deps -i -r
 ```

* Build
 ```
fakeroot dh binary
 ```

* Install package
 ```
cd ..
gdebi ./xrdp-xorg_*git*.deb
 ```
