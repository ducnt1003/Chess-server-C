#!/bin/bash
make clean > /dev/null 2>&1
make > /dev/null 2>&1
./client 127.0.0.1
