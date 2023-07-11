#!/bin/bash

cur_status=0 # 1 is up
while true; do
	if curl -s 'https://down.snappfood.ir/v1/status' | jq -r '.message' | grep -sq "snappfood is up."; then
		# It's up
		if [ $cur_status -eq 0 ]; then
			echo $(date) Snappfood is UP now.
			notify-send "Snappfood" "Up, It's up again."
			cur_status=1
		fi
	else
		if [ $cur_status -eq 1 ]; then
			echo $(date) Snappfood just went DOWN.
			notify-send "Snappfood" "DOWN, It's DOWN"
			cur_status=0
		fi
	fi
	sleep 3;
done

