#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fmi2.h"
#include "sim_support.h"
#include "FMURunner.h"

#ifdef MASTER_BUILD
#include "Master/MasterModel.hpp"
#endif

#ifdef SLAVEONE_BUILD
#include "SlaveOne/SlaveOne.hpp"
#endif

#ifdef SLAVETWO_BUILD
#include "SlaveTwo/SlaveTwo.hpp"
#endif

FMU fmu;

int main(int argc, char *argv[])
{
#ifdef MASTER_BUILD
    std::cout << "Master build" << std::endl;
    MasterModel master;
    master.start();
#endif

#ifdef SLAVEONE_BUILD
    std::cout << "Slave One build" << std::endl;
    SlaveOne slave1{&fmu};
    slave1.start();
#endif

#ifdef SLAVETWO_BUILD
    std::cout << "Slave Two build" << std::endl;
    SlaveTwo slave2{&fmu};
    slave2.start();
#endif

    return EXIT_SUCCESS;
}
