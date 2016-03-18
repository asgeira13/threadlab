#!/bin/sh
if [[ $# -le 3 ]]; then
    echo "Usage: $0 <K fairness> <number of vehicles> <number of pedestrians> <sleep step time for parser (0.1-0.5 is good)>"
else 
    ./traffic $1 $2 $3 | python visualize.py $4
fi
