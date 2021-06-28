# ai4robass2
Second assignment for the course of: Artificial Intelligence for Robotics II

Robotic Engineering, UNIGE

## Group B 
> Authors: 4070551, s5050695, s5103935, s5025439

## Compilation steps
### Popf-tif
Install dependencies to compile popf-tif:

    sudo apt-get install coinor-libcbc-dev coinor-libclp-dev coinor-libcoinutils-dev coinor-libosi-dev coinor-libcgl-dev

Read the readme inside `./popf-tif` to compile it.

### VisitSolver
Tnstall dependencies to compile the external solver, VisitSolver:

    sudo apt-get install liblapack-dev
    sudo apt-get install libblas-dev
    sudo apt-get install libboost-dev
    sudo apt-get install libarmadillo-dev  # this is the necessary one, the other 3 might be already installed

The module has some hard-coded path inside of it, relative to the files where waypoints, landmarks and region are stored. Before compiling, modify them in `./visits_module/src/VisitSolver.cpp`.

To compile the module run:
    
    ./visits_module/buildInstruction.txt


## Run the solver
Open the bash script `./run_planner.bash` with a text editor, change the necessary paths and names and run:
    
    ./run_planner.bash

## Filetree

```
    .
    ├── README.md
    ├── report.pdf
    ├── run_planner.bash
    ├── visits_domain
    │   ├── dom1.pddl
    │   ├── dom1_old.pddl
    │   ├── landmark.txt
    │   ├── libVisits.so.old
    │   ├── popf3-clp.old
    │   ├── prob1.pddl
    │   ├── prob1_old.pddl
    │   ├── region_poses
    │   └── waypoint.txt
    └── visits_module
        ├── build
        │   ├── Makefile
        │   ├── VisitMain
        │   ├── cmake_install.cmake
        │   └── libVisits.so
        └── src
            ├── CMakeLists.txt
            ├── ExternalSolver.cpp
            ├── ExternalSolver.h
            ├── VisitSolver.cpp
            ├── VisitSolver.h
            ├── buildInstruction.bash
            ├── buildInstruction.txt
            └── main.cpp

```