#!/bin/bash

#64bit buld script by Avian Overlord (aka LadyDi Andel)

#Use this as a starting point to compile the 64bit Linux
#version. It's here mostly as a guideline, but does work.

#Make sure we look into 64bit libraries... we use /usr/lib64
#first, as some systems (Suse, et al) still put 32bit libs
#in /usr/lib and 64bit in /usr/lib64, while Debian, et al,
#put the 64bit libraries in /usr/lib and the 32bit in /usr/lib32
export LD_LIBRARY_PATH=/usr/lib64:usr/lib

#clean out any previous build
python develop.py -m64 clean

#autoconfigi for a 64bit build (-m64)
python develop.py -m64 configure

#build it
python develop.py -m64 build

