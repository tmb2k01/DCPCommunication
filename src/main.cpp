/* -------------------------------------------------------------------------
 * main.c
 * Implements simulation of a single FMU instance
 * that implements the "FMI for Co-Simulation 2.0" interface.
 * Command syntax: see printHelp()
 * Simulates the given FMU from t = 0 .. tEnd with fixed step size h and
 * writes the computed solution to file 'result.csv'.
 * The CSV file (comma-separated values) may e.g. be plotted using
 * OpenOffice Calc or Microsoft Excel.
 * This program demonstrates basic use of an FMU.
 * Real applications may use advanced master algorithms to co-simulate
 * many FMUs, limit the numerical error using error estimation
 * and back-stepping, provide graphical plotting utilities, debug support,
 * and user control of parameter and start values, or perform a clean
 * error handling (e.g. call freeSlaveInstance when a call to the fmu
 * returns with error). All this is missing here.
 *
 * Revision history
 *  07.03.2014 initial version released in FMU SDK 2.0.0
 *
 * Free libraries and tools used to implement this simulator:
 *  - header files from the FMI specification
 *  - libxml2 XML parser, see http://xmlsoft.org
 *  - 7z.exe 4.57 zip and unzip tool, see http://www.7-zip.org
 * Author: Adrian Tirea
 * Copyright QTronic GmbH. All rights reserved.
 * -------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fmi2.h"
#include "sim_support.h"
#include "FMURunner.h"

#ifdef MASTER_BUILD
#include "Master/MasterModel.h"
#endif

#ifdef SLAVEONE_BUILD
#include "SlaveOne/SlaveOne.h"
#endif

#ifdef SLAVETWO_BUILD
#include "SlaveTwo/SlaveTwo.h"
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
