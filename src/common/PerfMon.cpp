#include "PerfMon.h"
#include <iostream>
#include <thread>
#include <unistd.h>

PerfMon::PerfMon() : m_start(std::clock())
{
    log("PerfMon started.");
}

PerfMon::~PerfMon() 
{
    log("PerfMon shutdown.");
}

void
PerfMon::log(const std::string& what)
{
    #ifdef DEBUG_CERR
    std::cerr 
        << "[PID:" << getpid() 
        << " TRD:" << std::this_thread::get_id() 
        << " TME:" << double(std::clock() - m_start) / CLOCKS_PER_SEC
        << "] \t" << what << std::endl;
    #endif
}