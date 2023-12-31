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
#include "SlaveBase.hpp"

const char *inputFileName = "../models/data.csv";

class SlaveOne : public SlaveBase
{
public:
    SlaveOne(const char *fmuFileName) : SlaveBase(fmuFileName)
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

        inInt = new int32_t;
        inReal = new float64_t;
    }

    ~SlaveOne()
    {
        delete inInt;
        delete inReal;
    };

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

            *inInt = integerValue;
            *inReal = realValue;
        }
    }

    void configure() override
    {
        SlaveBase::configure();

        outInt = manager->getOutput<int32_t *>(outInt_vr);
        outReal = manager->getOutput<float64_t *>(outReal_vr);
    }

    void initialize() override
    {
        openInputFile();
        SlaveBase::initialize();
    }

    void doStep(uint64_t steps) override
    {
        readInput();
        SlaveBase::doStep(steps);
    }

    void stateChanged(DcpState state) override
    {
        if (state == DcpState::ALIVE)
        {
            data.close();
            SlaveBase::stateChanged(state);
        }
    }

    SlaveDescription_t getSlaveDescription() override
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
    std::ifstream data;

    const char *const HOST = "127.0.0.1"; // Local
    // const char *const HOST = "172.20.0.3"; // Docker
    const int PORT = 8081;

    const uint32_t outInt_vr = 1;
    const uint32_t outReal_vr = 2;
};

#endif