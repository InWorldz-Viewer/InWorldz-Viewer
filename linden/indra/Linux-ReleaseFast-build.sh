#!/bin/bash

#ReleaseFast type build script by Avian Overlord (aka LadyDi Andel)

#Use this as a starting point to compile a different Linux
#build type. It's here mostly as a guideline, but does work.


#clean out any previous build
python develop.py -t ReleaseFast clean

#autoconfig
python develop.py -t ReleaseFast configure

#build it
python develop.py -t ReleaseFast build

