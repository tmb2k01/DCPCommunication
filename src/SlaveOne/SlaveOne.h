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

const char *fmuFileName = "../models/Proxy.fmu";
double tEnd = 10.0;
double h = 1;
int loggingOn = 0;
char csv_separator = ',';
char **categories = NULL;
int nCategories = 0;

class SlaveOne
{
public:
    SlaveOne(FMU *fmu) : stdLog(std::cout), runner{fmuFileName, tEnd, h, loggingOn, csv_separator, categories, nCategories, fmu}
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

        manager->addLogListener(
            std::bind(&OstreamLog::logOstream, stdLog, std::placeholders::_1));
        manager->setGenerateLogString(true);

        writeDcpSlaveDescription(getSlaveDescription(), "MSD1-Slave-Description.xml");
    }

    ~SlaveOne()
    {
        delete manager;
        delete udpDriver;
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

        // simulationTime +=
        //     ((double)numerator) / ((double)denominator) * ((double)steps);

        // double h = timeDiff / 10;

        // simulationTime += timeDiff;
        // currentStep += steps;

        runner.setIntInput(++inInt);
        runner.setRealInput(++inReal);

        runner.DoStep();

        runner.getIntOutput(outInt);
        runner.getRealOutput(outReal);

        runner.PrintStep(data_out_file);
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
        resolution.denominator = 10;
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
        slaveDescription.CapabilityFlags.canProvideLogOnRequest = true;
        slaveDescription.CapabilityFlags.canProvideLogOnNotification = true;

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
    OstreamLog stdLog;
    FMURunner runner;

    UdpDriver *udpDriver;
    const char *const HOST = "127.0.0.1";
    const int PORT = 8081;

    uint32_t numerator;
    uint32_t denominator;

    double simulationTime;
    uint64_t currentStep;
    const LogTemplate SIM_LOG = LogTemplate(
        1, 1, DcpLogLevel::LVL_INFORMATION,
        "[Time = %float64][step = %uint64 ] : pos = %float64, vel = %float64",
        {DcpDataType::float64, DcpDataType::uint64, DcpDataType::float64, DcpDataType::float64});

    int32_t inInt = 0;
    float64_t inReal = 0;

    int32_t *outInt;
    const uint32_t outInt_vr = 1;
    float64_t *outReal;
    const uint32_t outReal_vr = 2;

    FILE *data_out_file;
};

#endif