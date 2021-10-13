#!/bin/sh

export EXAMPLES_PATH="../../../examples"

cd $EXAMPLES_PATH

for EXAMPLE_NAME in $(ls)
do
  tput smso; echo $EXAMPLE_NAME; tput rmso
  cd $EXAMPLE_NAME/IDEs/Makefile
  make clean; make
  cd ../../..
done


#tput smso; echo hello, world; tput rmso
#tput blink; echo hello, world; tput sgr0
# Standout: smso and rmso
# Underline: smul and rmul
# Blink (yes!): blink
# Doublewide: swidm and rwidm
# Reverse: rev
# Cancel all: sgr0