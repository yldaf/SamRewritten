#pragma once
#include <ctime>
#include <string>

class PerfMon
{
private:
    std::clock_t m_start;
public:
    PerfMon();
    ~PerfMon();
    void log(const std::string& what);
};


