#!/bin/bash
path=/mnt/c/Users/rick/Desktop/visits/visits_domain/
domain=dom1.pddl
problem=prob1.pddl
extsolverinput=region_poses

popf3path=/mnt/c/Users/rick/Desktop/visits/popf-tif/planner/release/popf/popf3-clp
visitspath=/mnt/c/Users/rick/Desktop/visits/visits_module/build/libVisits.so

#run solver
$popf3path -x $path$domain $path$problem $visitspath $path$extsolverinput 
