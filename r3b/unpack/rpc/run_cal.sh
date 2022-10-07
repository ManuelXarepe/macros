#!/bin/bash

	echo Starting Precal calibration
	root -l -b -q RpcMapped2PreCalPar.C 
	wait
	echo Starting Cal calibration
	root -l -b -q RpcPreCal2CalPar.C 
	wait
        echo Starting Hit calibration
	root -l -b -q RpcCal2HitPar.C 
	wait
#        ./run_all.sh
	echo -e  ">>> Finished"

