#!/bin/bash

echo "Killing Existing Procs"
killall 360log
killall 360data
killall 360sync
killall 360comm
killall 360base

sudo kill -9 `cat /pids/twisted.pid`
sudo rm /pids/twisted.pid

echo "Quitting nginx"
sudo /usr/local/nginx/sbin/nginx -s quit