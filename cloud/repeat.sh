#!/bin/bash
# file:  repeat
# usgae: ./repeat TIMES [...ARGS]

n=0
times=$1
shift
while [[ $n -lt $times ]]; do
    $@
    n=$((n+1))
done