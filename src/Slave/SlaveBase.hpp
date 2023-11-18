#ifndef SLAVE_BASE_H_
#define SLAVE_BASE_H_

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

class SlaveBase
{
public:
    SlaveBase(const char *fmuFileName) : runner{fmuFileName, (1.0 / 5.0)}
    {
    }

    virtual ~SlaveBase()
    {
        delete manager;
        delete udpDriver;
    }

    virtual void configure()
    {
        simulationTime = 0;
        currentStep = 0;
    }

    virtual void initialize()
    {
        runner.InitializeFMU();
        runner.OpenFile();
    }

    virtual void doStep(uint64_t steps) = 0;

    void setTimeRes(const uint32_t numerator, const uint32_t denominator)
    {
        this->numerator = numerator;
        this->denominator = denominator;
    }

    virtual void start() { manager->start(); }

    virtual void stateChanged(DcpState state)
    {
        if (state == DcpState::ALIVE)
        {
            runner.CloseFile();
            runner.DisconnectFMU();
            std::exit(0);
        }
    }

    virtual SlaveDescription_t getSlaveDescription() = 0;

protected:
    FMURunner runner;

    DcpManagerSlave *manager;
    UdpDriver *udpDriver;

    uint32_t numerator = 1;
    uint32_t denominator = 5;

    double simulationTime;
    uint64_t currentStep;
};

#endif