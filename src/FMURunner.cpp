#include "FMURunner.h"

FMURunner::FMURunner(const char *fmuFileName, const double h, FMU *fmu)
    : m_fmu(fmu),
      m_fmuFileName(fmuFileName),
      m_h(h),
      callbacks({fmuLogger, calloc, free, NULL, m_fmu})
{
    inIntVr = 0;
    inRealVr = 0;
    outIntVr = 1;
    outRealVr = 1;
}

FMURunner::~FMURunner()
{
    DisconnectFMU();
}

int FMURunner::DoStep(double timeDiff)
{
    fmi2Flag = m_fmu->doStep(c, m_time, timeDiff, fmi2True);
    if (fmi2Flag == fmi2Discard)
    {
        fmi2Boolean b;
        // check if model requests to end simulation
        if (fmi2OK != m_fmu->getBooleanStatus(c, fmi2Terminated, &b))
        {
            return error("could not complete simulation of the model. getBooleanStatus return other than fmi2OK");
        }
        if (b == fmi2True)
        {
            return error("the model requested to end the simulation");
        }
        return error("could not complete simulation of the model");
    }
    if (fmi2Flag != fmi2OK)
        return error("could not complete simulation of the model");
    m_time += timeDiff;
    m_nSteps++;

    return 1;
}

void FMURunner::PrintStep(FILE *file)
{
    outputRow(m_fmu, c, m_time, file, m_separator, fmi2False);
}

int FMURunner::InitializeFMU()
{
    m_hh = m_h;
    m_time = m_tStart;
    loadFMU(m_fmuFileName, &m_tempPath);

    fmuResourceLocation = getTempResourcesLocation(m_tempPath);
    visible = fmi2False;
    vs = valueMissing;
    toleranceDefined = fmi2False;
    tolerance = 0;

    md = m_fmu->modelDescription;
    guid = getAttributeValue((Element *)md, att_guid);
    instanceName = getAttributeValue((Element *)getCoSimulation(md), att_modelIdentifier);
    c = m_fmu->instantiate(instanceName, fmi2CoSimulation, guid, fmuResourceLocation,
                           &callbacks, visible, m_loggingOn);
    free(fmuResourceLocation);

    if (!c)
        return error("could not instantiate model");

    if (m_nCategories > 0)
    {
        fmi2Flag = m_fmu->setDebugLogging(c, fmi2True, m_nCategories, m_categories);
        if (fmi2Flag > fmi2Warning)
        {
            return error("could not initialize model; failed FMI set debug logging");
        }
    }

    defaultExp = getDefaultExperiment(md);
    if (defaultExp)
        tolerance = getAttributeDouble(defaultExp, att_tolerance, &vs);
    if (vs == valueDefined)
    {
        toleranceDefined = fmi2True;
    }

    fmi2Flag = m_fmu->setupExperiment(c, toleranceDefined, tolerance, 0, fmi2False, 0);
    if (fmi2Flag > fmi2Warning)
    {
        return error("could not initialize model; failed FMI setup experiment");
    }
    fmi2Flag = m_fmu->enterInitializationMode(c);
    if (fmi2Flag > fmi2Warning)
    {
        return error("could not initialize model; failed FMI enter initialization mode");
    }
    fmi2Flag = m_fmu->exitInitializationMode(c);
    if (fmi2Flag > fmi2Warning)
    {
        return error("could not initialize model; failed FMI exit initialization mode");
    }

    return 1;
}

void FMURunner::DisconnectFMU()
{
    dlclose(m_fmu->dllHandle);
    freeModelDescription(m_fmu->modelDescription);
    if (m_categories)
        free(m_categories);
    deleteUnzippedFiles(&m_tempPath);
}

FILE *FMURunner::OpenFile()
{
    printf("FMU Simulator: run '%s' with step size h=%g, loggingOn=%d, csv separator='%c' ",
           m_fmuFileName, m_h, m_loggingOn, m_separator);
    printf("log categories={ ");
    for (int i = 0; i < m_nCategories; i++)
        printf("%s ", m_categories[i]);
    printf("}\n");

    FILE *file;
    // open result file
    if (!(file = fopen(RESULT_FILE, "w")))
    {
        printf("could not write %s because:\n", RESULT_FILE);
        printf("    %s\n", strerror(errno));
        return nullptr; // failure
    }

    // output solution for time t0
    outputRow(m_fmu, c, m_tStart, file, m_separator, fmi2True);  // output column names
    outputRow(m_fmu, c, m_tStart, file, m_separator, fmi2False); // output values

    return file;
}

void FMURunner::CloseFile(FILE *file)
{
    // end simulation
    m_fmu->terminate(c);
    m_fmu->freeInstance(c);
    fclose(file);

    // print simulation summary
    printf("Simulation terminated successful\n");
    printf("  steps ............ %d\n", m_nSteps);
    printf("  fixed step size .. %g\n", m_h);
    printf("CSV file '%s' written\n", RESULT_FILE);
}

void FMURunner::setIntInput(int const &i)
{
    m_fmu->setInteger(c, &inIntVr, 1, &i);
}

void FMURunner::setRealInput(double const &i)
{
    m_fmu->setReal(c, &inRealVr, 1, &i);
}

void FMURunner::getIntOutput(int *i)
{
    m_fmu->getInteger(c, &outIntVr, 1, i);
}

void FMURunner::getRealOutput(double *i)
{
    m_fmu->getReal(c, &outRealVr, 1, i);
}