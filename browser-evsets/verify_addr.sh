#!/usr/bin/env bash

declare -A allocated

while true; do

	while read line; do
		if [[ $line =~ "Yo fam, found me a set" ]]; then
			break;
		fi
		if [[ $line == *"Prepar"* ]]; then
			echo $line
		fi
		if [[ $line == *"HeapNumber"* ]]; then
			echo $line
		fi
		if [[ $line == *"Calibration error"* ]]; then
			echo $line
		fi
	done

	# find right pid checking large allocated buffer
	base=""
	pids=$(ps aux | grep 'chrome/chrome --type=renderer' | awk '{print $2}')
	for p in $pids; do
		bases=$(pmap $p | grep '131072K' | awk '{print $1}')
		#bases=$(pmap $p | grep '132096K' | awk '{print $1}')
		if [ ! -z "$bases" ]; then
			pid=$p
			break;
		fi
	done

	# find right allocated buffer if no gc yet
	for c in $bases; do
		if [ -z "${allocated[c]}" ]; then
			allocated+=(["$c"]=1) # add element to avoid repeat
			base="0x$c"
			break;
		fi
	done
	if [ -z "$base" ]; then
		echo "[!] Error"
		exit 1
	fi
	echo "PID: $pid"
	echo "Virtual base: $base"

	for c in $bases; do
		base="0x$c"
		echo "Yo, I got base $base"
	done

	conflict=0

	while read line; do
			if [[ $line =~ "Creating conflict set..." ]]; then
				conflict=1
			elif [[ $line =~ "Eviction set:" ]]; then
				addresses="$(echo $line | cut -d: -f 3 | cut -d\> -f 1 | cut -d\" -f 1 | tr ',' ' ')"
				echo "Physical addresses:"
		for c in $bases; do
				base="0x$c"
				vaddrs=$(for o in $addresses; do printf '%x ' $(($base+$o)); done)
				# needs sudo to work
				sudo ./virt_to_phys $pid $vic $vaddrs
				echo "============================================"
		done
			elif [[ $line =~ "EOF" ]]; then
				break;
			fi
	done

done
