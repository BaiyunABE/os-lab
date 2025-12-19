#!/bin/bash
make
./msg/start.sh
./shm/start.sh
./pipe/start.sh
make clean