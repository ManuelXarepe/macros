#!/bin/bash

for i in {0001..0130};
do
    /u/land/klenze/califa_merge_stitch/califa_merge_stitch.bash /lustre/r3b/202205_s522/lmd/main0127_$i.lmd ""  /u/land/mxarepe/unpkd_data/analysis/sticth_califa/main_TS_0127_$i.lmd
    wait
done

