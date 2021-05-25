#!/bin/bash
path=/mnt/c/Users/rick/Desktop/visits/visits_domain/
domain=dom1.pddl
problem=prob1.pddl
now="$(date +"%s")"
#echo $now

#save plan 
./visit_module/build/./VisitMain problem parameters plan $path$domain -f $path$problem -sp 