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
#include "SlaveOne/SlaveOne.hpp"
#endif

#ifdef SLAVETWO_BUILD
#include "SlaveTwo/SlaveTwo.hpp"
#endif

int main(int argc, char *argv[])
{
#if defined(MASTER_BUILD)
    std::cout << "Master build" << std::endl;
    MasterModel component;
#elif defined(SLAVEONE_BUILD)
    std::cout << "Slave One build" << std::endl;
    SlaveOne component{&fmu};
#elif defined(SLAVETWO_BUILD)
    std::cout << "Slave Two build" << std::endl;
    SlaveTwo component{&fmu};
#else
    std::cerr << "Component doesn't defined.";
    return 1;
#endif
    component.start();
    return 0;
}
