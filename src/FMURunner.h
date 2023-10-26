#ifndef FMURUNNER_H
#define FMURUNNER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fmi2.h"
#include "sim_support.h"

class FMURunner
{
private:
    FMU *m_fmu;
    const char *m_fmuFileName;

    double m_time;
    double m_tStart = 0;
    double m_tEnd;

    int m_nSteps = 0;
    double m_hh;
    double m_h;

    int m_loggingOn;
    char m_separator;
    const char **m_categories;
    int m_nCategories;

    fmi2ValueReference intVr;
    fmi2ValueReference realVr;

    char *fmuResourceLocation;
    fmi2Boolean visible;
    fmi2CallbackFunctions callbacks;
    Element *defaultExp;
    ValueStatus vs;
    fmi2Boolean toleranceDefined;
    fmi2Real tolerance;

    ModelDescription *md;
    const char *guid;
    const char *instanceName;
    fmi2Component c;
    fmi2Status fmi2Flag;

public:
    FMURunner(const char *fmuFileName, const double h,
              const int logging, const char csv_separator, const char **categories, const int nCategories, FMU *fmu);

    ~FMURunner();
    int InitializeFMU();

    FILE *OpenFile();
    void CloseFile(FILE *file);

    int DoStep(double timeDiff);
    void PrintStep(FILE *file);

    void setIntInput(int const &);
    void setRealInput(double const &);
    void getIntOutput(int *);
    void getRealOutput(double *);
};

#endif