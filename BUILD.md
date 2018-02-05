# Ubuntu packaging recipe for xorgxrdp

Tested on Ubuntu 16.04

* Create an empty working directory
 ```
mkdir ~/xorgxrdp-build
cd ~/xorgxrdp-build
 ```

* Build xorgxrdp
 ```
apt install git devscripts equivs gdebi-core
git clone -b ubuntu-devel https://github.com/ubuntu-xrdp/xorgxrdp.git
cd xorgxrdp
mk-build-deps -i -r
debuild binary
 ```

* Install built package
 ```
cd ..
gdebi ./xrdp-xorg_*git*.deb
 ```

