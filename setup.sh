#!/bin/bash

perl -pi -e "s/python/python2.4/" /usr/bin/yum
mv /usr/bin/python /usr/bin/python2.4

yum install gcc gcc-c++ gdb mysql mysql-server mysql-devel freetype freetype-devel jpeg libjpeg-devel libpng libpng-devel zlib zlib-devel screen

rm -rfd /Life/deps
mkdir /Life/deps

# Python

cd /Life/deps
rm -rfd Python-2.5.4
wget http://www.python.org/ftp/python/2.5.4/Python-2.5.4.tgz
tar xvf Python-2.5.4.tgz
rm -f Python-2.5.4.tgz
cd Python-2.5.4
./configure
make
make install
ln -s /usr/local/bin/python /usr/bin/python

cd /Life/deps
rm -rfd Python-2.5.4

# Cheetah

cd /Life/deps
rm -rfd Cheetah-2.4.1
wget http://superb-west.dl.sourceforge.net/sourceforge/cheetahtemplate/Cheetah-2.4.1.tar.gz
tar xvf Cheetah-2.4.1.tar.gz
rm -f Cheetah-2.4.1.tar.gz
cd Cheetah-2.4.1
python setup.py install

cd /Life/deps
rm -rfd Cheetah-2.4.1

# Twisted

cd /Life/deps
rm -rfd Twisted-8.2.0
wget http://tmrc.mit.edu/mirror/twisted/Twisted/8.2/Twisted-8.2.0.tar.bz2
tar xvf Twisted-8.2.0.tar.bz2
rm -f Twisted-8.2.0.tar.bz2
cd Twisted-8.2.0
python setup.py install

cd /Life/deps
rm -rfd Twisted-8.2.0

# Zope

cd /Life/deps
rm -rfd zope.interface-3.3.0
wget http://www.zope.org/Products/ZopeInterface/3.3.0/zope.interface-3.3.0.tar.gz
tar xvf zope.interface-3.3.0.tar.gz
rm -f zope.interface-3.3.0.tar.gz
cd zope.interface-3.3.0
python setup.py install

cd /Life/deps
rm -rfd zope.interface-3.3.0

# MySQL-Python

cd /Life/deps
rm -rfd MySQL-python-1.2.2
wget http://softlayer.dl.sourceforge.net/sourceforge/mysql-python/MySQL-python-1.2.2.tar.gz
tar xvf MySQL-python-1.2.2.tar.gz
rm -f MySQL-python-1.2.2.tar.gz
cd MySQL-python-1.2.2
python setup.py install

cd /Life/deps
rm -rfd MySQL-python-1.2.2

# TwistedWeb

cd /Life/deps
rm -rfd TwistedWeb2-8.1.0
wget http://tmrc.mit.edu/mirror/twisted/Web2/8.1/TwistedWeb2-8.1.0.tar.bz2
tar xvf TwistedWeb2-8.1.0.tar.bz2
rm -f TwistedWeb2-8.1.0.tar.bz2
cd TwistedWeb2-8.1.0
python setup.py install

cd /Life/deps
rm -rfd TwistedWeb2-8.1.0

# PyCrypto

cd /Life/deps
rm -rfd pycrypto-2.0.1
wget http://www.amk.ca/files/python/crypto/pycrypto-2.0.1.tar.gz
tar xvf pycrypto-2.0.1.tar.gz
rm -f pycrypto-2.0.1.tar.gz
cd pycrypto-2.0.1
python setup.py install

cd /Life/deps
rm -rfd pycrypto-2.0.1

# PIL

cd /Life/deps
rm -rfd Imaging-1.1.6
wget http://effbot.org/downloads/Imaging-1.1.6.tar.gz
tar xvf Imaging-1.1.6.tar.gz
rm -f Imaging-1.1.6.tar.gz
cd Imaging-1.1.6
python setup.py build_ext -i
python selftest.py
python setup.py install

cd /Life/deps
rm -rfd Imaging-1.1.6

# SimpleJSON

easy_install simplejson

# TwittyTwister

cd /Life/deps
wget http://github.com/dustin/twitty-twister/zipball/master --output-document=dustin-twitty.zip
unzip dustin-twitty.zip
rm -f dustin-twitty.zip
cd dustin-twitty*
python setup.py install

cd /Life/deps
rm -rfd dustin-twitty*

# OAuth

easy_install oauth

# Epsilon

easy_install python-epsilon

# BDB

cd /Life/deps
rm -rfd db-4.7.25
wget http://download.oracle.com/berkeley-db/db-4.7.25.tar.gz
tar xvf db-4.7.25.tar.gz
rm -f db-4.7.25.tar.gz
cd db-4.7.25/build_unix
../dist/configure --enable-cxx
make
make install

cd /Life/deps
rm -rfd db-4.7.25

# libEV

cd /Life/deps
rm -rfd libev-3.8
wget http://dist.schmorp.de/libev/Attic/libev-3.8.tar.gz
tar xvf libev-3.8.tar.gz
rm -f libev-3.8.tar.gz
cd libev-3.8
./configure
make
make install

cd /Life/deps
rm -rfd libev-3.8

# nginx

cd /Life/deps
rm -rfd nginx-0.7.61
wget http://sysoev.ru/nginx/nginx-0.7.61.tar.gz
tar xvf nginx-0.7.61.tar.gz
rm -f nginx-0.7.61.tar.gz
cd nginx-0.7.61
./configure --without-http_rewrite_module
make
make install

cd /Life/deps
rm -rfd nginx-0.7.61

rm -f /usr/local/nginx/conf/nginx.conf
cp /Life/360io/nginx.conf /usr/local/nginx/conf/nginx.conf

# build directories

mkdir /Life/360io/server/base/build /Life/360io/server/comm/build /Life/360io/server/data/build /Life/360io/server/sync/build

# db directory

mkdir /Life/360io/db
chmod 777 /Life/360io/db

# shared object libraries

echo "/usr/lib" >> /etc/ld.so.conf
echo "/usr/local/lib" >> /etc/ld.so.conf
echo "/usr/local/BerkeleyDB.4.7/lib" >> /etc/ld.so.conf
/sbin/ldconfig

# make sure that we have localhost

echo "127.0.0.1 localhost"

# start mysqld for the first time

service mysqld start

# create user qorporation / db io360