
#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>

#include <unistd.h>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>

#include "utils.h"

namespace crx {

std::string get_mem() {
    unsigned long long rss = 0, vss = 0;
    rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    rss = usage.ru_maxrss * 1024.0;
    std::stringstream str;
    str << "RM: " << (rss / 1024.0 / 1024.0) << " MB, VM: " << (vss / 1024.0 / 1024.0) << " MB";
    /*(r_usage.ru_maxrss / 1024.0 / 1024.0) << " MB"; */
    return str.str();
}

void process_mem_usage(double& vm_usage, double& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}

}
