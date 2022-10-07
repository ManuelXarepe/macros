#!/bin/bash

for i in {0041..0130};
do
	echo main_TS_0127_$i
	sed -i "s/main_TS_0127_0130/main_TS_0127_$i/g" unpack_all.C
	#sed -i "s/main_TS_0127_0130/main_TS_0127_$i/g" rpc_online.C
	root -l -b unpack_all.C
	sed -i "s/main_TS_0127_$i/main_TS_0127_0130/g" unpack_all.C
	#sed -i "s/main_TS_0127_$i/main_TS_0127_0130/g" rpc_online.C
done
	wait
	echo -e  ">>> Finished"

