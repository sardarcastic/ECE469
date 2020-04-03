#!/bin/bash


cd os/
make clean
make


cd ../apps/single_fork_call
make clean
make
make run
