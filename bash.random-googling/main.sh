#!/bin/bash

for i in $(seq 1 20); do
    dig=$((10 + $RANDOM % 10))
    rand=$(openssl rand -hex $dig)
    echo "Generated String: $rand";
    brave "https://www.google.com/search?q=$rand"
    sleep 2
done

