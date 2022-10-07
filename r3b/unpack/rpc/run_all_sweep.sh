#!/bin/bash

for i in {0165..0186};
do
	echo main$i
	sed -i "s/main0165/main$i/g" rpc_online.C
	root -l -b rpc_online.C
	sed -i "s/main$i/main0165/g" rpc_online.C
done
	wait
	echo -e  ">>> Finished"

