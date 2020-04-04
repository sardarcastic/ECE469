#!/bin/bash
rm testoutput

cd os/
make clean
make


cd ../apps/single_fork_call
make clean
make
make run > testoutput
mv testoutput ../../
cd ../..
less testoutput
