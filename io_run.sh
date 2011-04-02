#!/bin/bash

LD_RUN_PATH=/usr/local/lib:/usr/local/BerkeleyDB.4.7/lib
export LD_RUN_PATH

sudo kill -9 `cat /pids/twisted.pid`
sudo rm /pids/twisted.pid

echo "Quitting nginx"
sudo /usr/local/nginx/sbin/nginx -s quit

echo "Killing Existing Procs"
killall 360log
killall 360data
killall 360sync
killall 360comm
killall 360base

cd /Life/360io

echo "Running 360log"
#valgrind --leak-check=yes server/data/build/360data &
cd /Life/360io/server/log/build
./360log &> /Life/360io/360log.log &

echo "Running 360data"
#valgrind --leak-check=yes server/data/build/360data &
cd /Life/360io/server/data/build
./360data &> /Life/360io/360data.log &

echo "Running 360sync"
#valgrind --leak-check=yes server/sync/build/360sync &
cd /Life/360io/server/sync/build
./360sync &> /Life/360io/360sync.log &

echo "Running 360comm"
#valgrind --leak-check=yes server/comm/build/360comm &
cd /Life/360io/server/comm/build
./360comm &> /Life/360io/360comm.log &

echo "Sleeping to give daemons warm up period"
sleep 1

echo "Running 360base"
#valgrind --leak-check=yes server/base/build/360base &
cd /Life/360io/server/base/build
./360base &> /Life/360io/360base.log &

echo "Running 360service"
cd /Life/360io
./io_service.sh &> /Life/360io/360service.log &

echo "Starting nginx"
sudo /usr/local/nginx/sbin/nginx
