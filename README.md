# ai4robass2
Second assignment for the course of: Artificial Intelligence for Robotics II

Robotic Engineering, UNIGE

## Group B 
> Authors: 4070551, s5050695, s5103935, s5025439

## Compilation steps
### popf-tif
install dependencies to compile popf-tif

    sudo apt-get install coinor-libcbc-dev coinor-libclp-dev coinor-libcoinutils-dev coinor-libosi-dev coinor-libcgl-dev

read the readme inside `./popf-tif` to compile it

### visitsolver
install dependencies to compile visitsolver

    sudo apt-get install liblapack-dev
    sudo apt-get install libblas-dev
    sudo apt-get install libboost-dev
    sudo apt-get install libarmadillo-dev  # this is the necessary one, the other 3 might be already installed

run `./visits_module/buildInstruction.txt` to compile the module

## Run the solver
open the bash script `./run_planner.bash` with a text editor, change the necessary paths and names and run it

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