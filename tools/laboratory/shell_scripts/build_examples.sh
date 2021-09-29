#!/bin/sh

export EXAMPLES_PATH="../../../examples"

cd $EXAMPLES_PATH

for EXAMPLE_NAME in blinky_1 blinky_2 blinky_3 oblique parse_http platform_test task_join task_join_wto tower wall_clock
do
  echo $EXAMPLE_NAME
  cd $EXAMPLE_NAME/IDEs/Makefile
  make clean; make
  cd ../../..
done

