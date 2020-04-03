#! /bin/bash
cd ../../os
make clean
make
cd ../apps/example
make clean
make
make run > testoutput
less testoutput
