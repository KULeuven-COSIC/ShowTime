#!/bin/bash

for i in {1..100}; do
	echo $i >&2
	./arch_reorder
done
