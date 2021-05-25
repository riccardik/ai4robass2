# ai4robass2


install dependencies to compile popf-tif

    sudo apt-get install coinor-libcbc-dev coinor-libclp-dev coinor-libcoinutils-dev coinor-libosi-dev coinor-libcgl-dev

read the readme inside `./popf-tif` to compile it

install dependencies to compile visitsolver

    sudo apt-get install liblapack-dev
    sudo apt-get install libblas-dev
    sudo apt-get install libboost-dev
    sudo apt-get install libarmadillo-dev  # this is the necessary one, the other 3 might be already installed

run `./visits_module/buildInstruction.txt` to compile the module

