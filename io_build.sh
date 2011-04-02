#!/bin/bash

cd /Life/360io/server/base
python make.py

cd /Life/360io/server/comm
python make.py

cd /Life/360io/server/data
python make.py

cd /Life/360io/server/sync
python make.py

cd /Life/360io/server/log
python make.py

cd /Life/360io
