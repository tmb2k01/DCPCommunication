#ifndef SLAVE_TWO_H_
#define SLAVE_TWO_H_

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
const int loggingOn = 0;
const char csv_separator = ',';
const char **categories = NULL;
const int nCategories = 0;

class SlaveTwo
{
public:
    SlaveTwo(FMU *fmu) : runner{fmuFileName, ((double)numerator / (double)denominator), loggingOn, csv_separator, categories, nCategories, fmu}
    {
        udpDriver = new UdpDriver(HOST, PORT);
        manager = new DcpManagerSlave(getSlaveDescription(), udpDriver->getDcpDriver());
        manager->setInitializeCallback<SYNC>(
            std::bind(&SlaveTwo::initialize, this));
        manager->setConfigureCallback<SYNC>(
            std::bind(&SlaveTwo::configure, this));
        manager->setSynchronizingStepCallback<SYNC>(
            std::bind(&SlaveTwo::doStep, this, std::placeholders::_1));
        manager->setSynchronizedStepCallback<SYNC>(
            std::bind(&SlaveTwo::doStep, this, std::placeholders::_1));
        manager->setRunningStepCallback<SYNC>(
            std::bind(&SlaveTwo::doStep, this, std::placeholders::_1));
        manager->setTimeResListener<SYNC>(std::bind(&SlaveTwo::setTimeRes, this,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));
        manager->setStateChangedListener<SYNC>(
            std::bind(&SlaveTwo::stateChanged, this, std::placeholders::_1));

        writeDcpSlaveDescription(getSlaveDescription(), "MSD2-Slave-Description.xml");
    }

    ~SlaveTwo()
    {
        delete manager;
        delete udpDriver;

        runner.CloseFile(data_out_file);
    }

    void configure()
    {
        simulationTime = 0;
        currentStep = 0;

        runner.InitializeFMU();
        data_out_file = runner.OpenFile();

        inInt = manager->getInput<int32_t *>(inInt_vr);
        inReal = manager->getInput<float64_t *>(inReal_vr);
    }

    void initialize()
    {
    }

    void doStep(uint64_t steps)
    {
        float64_t timeDiff =
            ((double)numerator) / ((double)denominator) * ((double)steps);

        runner.setIntInput((*inInt));
        runner.setRealInput((*inReal));

        runner.DoStep(timeDiff);

        runner.getIntOutput(&outInt);
        runner.getRealOutput(&outReal);

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
        SlaveDescription_t slaveDescription = make_SlaveDescription(1, 0, "dcpslave", "b3663a74-5e59-11ec-bf63-0242ac130003");
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
        slaveDescription.CapabilityFlags.canHandleReset = true;
        slaveDescription.CapabilityFlags.canHandleVariableSteps = true;
        slaveDescription.CapabilityFlags.canMonitorHeartbeat = false;
        slaveDescription.CapabilityFlags.canAcceptConfigPdus = true;
        slaveDescription.CapabilityFlags.canProvideLogOnRequest = false;
        slaveDescription.CapabilityFlags.canProvideLogOnNotification = false;

        std::shared_ptr<CommonCausality_t> caus_inInt = make_CommonCausality_ptr<int32_t>();
        caus_inInt->Int32->start = std::make_shared<std::vector<int32_t>>();
        caus_inInt->Int32->start->push_back(0);
        slaveDescription.Variables.push_back(make_Variable_input("inInt", inInt_vr, caus_inInt));
        std::shared_ptr<CommonCausality_t> caus_inReal = make_CommonCausality_ptr<float64_t>();
        caus_inReal->Float64->start = std::make_shared<std::vector<float64_t>>();
        caus_inReal->Float64->start->push_back(0.0);
        slaveDescription.Variables.push_back(make_Variable_input("inReal", inReal_vr, caus_inReal));

        return slaveDescription;
    }

private:
    DcpManagerSlave *manager;
    FMURunner runner;

    UdpDriver *udpDriver;
    const char *const HOST = "127.0.0.1";
    const int PORT = 8082;

    uint32_t numerator;
    uint32_t denominator;

    double simulationTime;
    uint64_t currentStep;

    int32_t *inInt;
    const uint32_t inInt_vr = 1;
    float64_t *inReal;
    const uint32_t inReal_vr = 2;

    int32_t outInt;
    float64_t outReal;

    FILE *data_out_file;
};

#endif