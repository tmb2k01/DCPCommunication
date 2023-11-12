#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(SLAVEONE_BUILD) || defined(SLAVETWO_BUILD)
#include "fmi2.h"
#include "sim_support.h"
#include "FMURunner.h"

FMU fmu;
#endif

#ifdef MASTER_BUILD
#include "Master/MasterModel.hpp"
#endif

#ifdef SLAVEONE_BUILD
#include "Slave/SlaveOne.hpp"
const char *fmuFileName = "../models/Model1.fmu";
#endif

#ifdef SLAVETWO_BUILD
#include "Slave/SlaveTwo.hpp"
const char *fmuFileName = "../models/Model2.fmu";
#endif

int main(int argc, char *argv[])
{
#if defined(MASTER_BUILD)
    std::cout << "Master build" << std::endl;
    MasterModel component;
#elif defined(SLAVEONE_BUILD)
    std::cout << "Slave One build" << std::endl;
    SlaveOne component{&fmu, fmuFileName};
#elif defined(SLAVETWO_BUILD)
    std::cout << "Slave Two build" << std::endl;
    SlaveTwo component{&fmu, fmuFileName};
#else
    std::cerr << "Component doesn't defined.";
    return 1;
#endif
    component.start();
    return 0;
}
