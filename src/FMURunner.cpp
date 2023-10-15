#include "FMURunner.h"

FMURunner::FMURunner(const char *fmuFileName, double tEnd, double h,
                     int logging, char csv_separator, char **categories, int nCategories, FMU *fmu)
    : m_fmu(fmu),
      m_fmuFileName(fmuFileName),
      m_tEnd(tEnd),
      m_h(h),
      m_loggingOn(logging),
      m_separator(csv_separator),
      m_categories(categories),
      m_nCategories(nCategories),
      callbacks({fmuLogger, calloc, free, NULL, m_fmu})
{
    loadFMU(fmuFileName);
    m_hh = m_h;
    inInt = 0;
    inReal = 0;
}

FMURunner::~FMURunner()
{
    dlclose(m_fmu->dllHandle);
    freeModelDescription(m_fmu->modelDescription);
    if (m_categories)
        free(m_categories);
    deleteUnzippedFiles();
}

int FMURunner::RunSimulation()
{
    if (InitializeFMU() == 0)
    {
        return 0;
    }

    FILE *file = OpenFile();
    if (file == nullptr)
    {
        return 0;
    }

    // enter the simulation loop
    m_time = m_tStart;
    while (m_time < m_tEnd)
    {
        inInt++;
        inReal++;
        m_fmu->setInteger(c, &intVr, 1, &inInt);
        m_fmu->setReal(c, &realVr, 1, &inReal);
        if (DoStep() == 0)
        {
            return 0;
        }
        outputRow(m_fmu, c, m_time, file, m_separator, fmi2False); // output values for this step
    }

    CloseFile(file);
    return 1;
}

int FMURunner::DoStep()
{
    if (m_h > m_tEnd - m_time)
    {
        m_hh = m_tEnd - m_time;
    }
    fmi2Flag = m_fmu->doStep(c, m_time, m_hh, fmi2True);
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
    m_time += m_hh;
    m_nSteps++;

    return 1;
}

int FMURunner::InitializeFMU()
{
    fmuResourceLocation = getTempResourcesLocation();
    visible = fmi2False;
    vs = valueMissing;
    toleranceDefined = fmi2False; // true if model description define tolerance
    tolerance = 0;                // used in setting up the experiment

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

    fmi2Flag = m_fmu->setupExperiment(c, toleranceDefined, tolerance, 0, fmi2True, m_tEnd);
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

FILE *FMURunner::OpenFile()
{
    printf("FMU Simulator: run '%s' from t=0..%g with step size h=%g, loggingOn=%d, csv separator='%c' ",
           m_fmuFileName, m_tEnd, m_h, m_loggingOn, m_separator);
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
    printf("Simulation from %g to %g terminated successful\n", m_tStart, m_tEnd);
    printf("  steps ............ %d\n", m_nSteps);
    printf("  fixed step size .. %g\n", m_h);
    printf("CSV file '%s' written\n", RESULT_FILE);
}