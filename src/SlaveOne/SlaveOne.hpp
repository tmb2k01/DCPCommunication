#ifndef SLAVE_ONE_H_
#define SLAVE_ONE_H_

#include <dcp/helper/Helper.hpp>
#include <dcp/log/OstreamLog.hpp>
#include <dcp/logic/DcpManagerSlave.hpp>
#include <dcp/model/pdu/DcpPduFactory.hpp>
#include <dcp/driver/ethernet/udp/UdpDriver.hpp>
#include <dcp/xml/DcpSlaveDescriptionWriter.hpp>

#include <cstdint>
#include <cstdio>
#include <stdarg.h>
#include <thread>
#include <cmath>
#include "../FMURunner.h"

const char *fmuFileName = "../models/Model1.fmu";
const char *inputFileName = "../models/data.csv";
const int loggingOn = 0;
const char csv_separator = ',';
const char **categories = NULL;
const int nCategories = 0;

class SlaveOne
{
public:
    SlaveOne(FMU *fmu) : runner{fmuFileName, ((double)numerator / (double)denominator), loggingOn, csv_separator, categories, nCategories, fmu}
    {
        udpDriver = new UdpDriver(HOST, PORT);
        manager = new DcpManagerSlave(getSlaveDescription(), udpDriver->getDcpDriver());
        manager->setInitializeCallback<SYNC>(
            std::bind(&SlaveOne::initialize, this));
        manager->setConfigureCallback<SYNC>(
            std::bind(&SlaveOne::configure, this));
        manager->setSynchronizingStepCallback<SYNC>(
            std::bind(&SlaveOne::doStep, this, std::placeholders::_1));
        manager->setSynchronizedStepCallback<SYNC>(
            std::bind(&SlaveOne::doStep, this, std::placeholders::_1));
        manager->setRunningStepCallback<SYNC>(
            std::bind(&SlaveOne::doStep, this, std::placeholders::_1));
        manager->setTimeResListener<SYNC>(std::bind(&SlaveOne::setTimeRes, this,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));
        manager->setStateChangedListener<SYNC>(
            std::bind(&SlaveOne::stateChanged, this, std::placeholders::_1));

        writeDcpSlaveDescription(getSlaveDescription(), "MSD1-Slave-Description.xml");
        openInputFile();
    }

    ~SlaveOne()
    {
        delete manager;
        delete udpDriver;
    }

    void openInputFile()
    {
        data.open(inputFileName);
        if (!data.is_open())
        {
            std::cerr << "Failed to open the file." << std::endl;
            std::exit(1);
        }
    }

    void readInput()
    {
        std::string line;
        if (std::getline(data, line))
        {
            int integerValue;
            double realValue;

            std::istringstream iss(line);
            char comma;
            if (!(iss >> integerValue >> comma >> realValue))
            {
                std::cerr << "Error reading values from line: " << line << std::endl;
                return;
            }

            inInt = integerValue;
            inReal = realValue;
        }
    }

    void configure()
    {
        simulationTime = 5;
        currentStep = 0;

        runner.InitializeFMU();
        data_out_file = runner.OpenFile();

        outInt = manager->getOutput<int32_t *>(outInt_vr);
        outReal = manager->getOutput<float64_t *>(outReal_vr);
    }

    void initialize()
    {
    }

    void doStep(uint64_t steps)
    {

        float64_t timeDiff =
            ((double)numerator) / ((double)denominator) * ((double)steps);

        readInput();
        runner.setIntInput(inInt);
        runner.setRealInput(inReal);

        runner.DoStep(timeDiff);

        runner.getIntOutput(outInt);
        runner.getRealOutput(outReal);

        runner.PrintStep(data_out_file);

        simulationTime += timeDiff;
        currentStep += steps;
    }

    void setTimeRes(const uint32_t numerator, const uint32_t denominator)
    {
        this->numerator = numerator;
        this->denominator = denominator;
    }

    void start() { manager->start(); }

    void stateChanged(DcpState state)
    {
        if (state == DcpState::ALIVE)
        {
            runner.CloseFile(data_out_file);
            runner.~FMURunner();
            std::exit(0);
        }
    }

    SlaveDescription_t getSlaveDescription()
    {
        SlaveDescription_t slaveDescription = make_SlaveDescription(1, 0, "dcpslave", "bcb912fe-5e59-11ec-bf63-0242ac130002");
        slaveDescription.OpMode.SoftRealTime = make_SoftRealTime_ptr();
        Resolution_t resolution = make_Resolution();
        resolution.numerator = 1;
        resolution.denominator = 5;
        slaveDescription.TimeRes.resolutions.push_back(resolution);
        slaveDescription.TransportProtocols.UDP_IPv4 = make_UDP_ptr();
        slaveDescription.TransportProtocols.UDP_IPv4->Control =
            make_Control_ptr(HOST, PORT);
        ;
        slaveDescription.TransportProtocols.UDP_IPv4->DAT_input_output = make_DAT_ptr();
        slaveDescription.TransportProtocols.UDP_IPv4->DAT_input_output->availablePortRanges.push_back(
            make_AvailablePortRange(2048, 65535));
        slaveDescription.TransportProtocols.UDP_IPv4->DAT_parameter = make_DAT_ptr();
        slaveDescription.TransportProtocols.UDP_IPv4->DAT_parameter->availablePortRanges.push_back(
            make_AvailablePortRange(2048, 65535));
        slaveDescription.CapabilityFlags.canAcceptConfigPdus = true;
        slaveDescription.CapabilityFlags.canHandleReset = false;
        slaveDescription.CapabilityFlags.canHandleVariableSteps = true;
        slaveDescription.CapabilityFlags.canMonitorHeartbeat = false;
        slaveDescription.CapabilityFlags.canAcceptConfigPdus = true;
        slaveDescription.CapabilityFlags.canProvideLogOnRequest = false;
        slaveDescription.CapabilityFlags.canProvideLogOnNotification = false;

        std::shared_ptr<Output_t> caus_outInt = make_Output_ptr<int32_t>();
        caus_outInt->Int32->start = std::make_shared<std::vector<int32_t>>();
        caus_outInt->Int32->start->push_back(0);
        slaveDescription.Variables.push_back(make_Variable_output("outInt", outInt_vr, caus_outInt));
        std::shared_ptr<Output_t> caus_outReal = make_Output_ptr<float64_t>();
        caus_outReal->Float64->start = std::make_shared<std::vector<float64_t>>();
        caus_outReal->Float64->start->push_back(0.0);
        slaveDescription.Variables.push_back(make_Variable_output("outReal", outReal_vr, caus_outReal));

        return slaveDescription;
    }

private:
    DcpManagerSlave *manager;
    FMURunner runner;
    std::ifstream data;

    UdpDriver *udpDriver;
    const char *const HOST = "127.0.0.1";
    const int PORT = 8081;

    uint32_t numerator;
    uint32_t denominator;

    double simulationTime;
    uint64_t currentStep;

    int32_t inInt = 0;
    float64_t inReal = 0;

    int32_t *outInt;
    const uint32_t outInt_vr = 1;
    float64_t *outReal;
    const uint32_t outReal_vr = 2;

    FILE *data_out_file;
};

#endif