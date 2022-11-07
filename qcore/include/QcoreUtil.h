#ifndef Header_qcore_Util
#define Header_qcore_Util

#include <exception>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "Qcore_API.h"

namespace qcore
{
   namespace util
   {
      /** Custom exception to be used whithin qcore framework */
      class QCODE_API Exception : public std::exception
      {
      public:
         explicit Exception(const std::string &what) : exception(), cause(what) {}
         virtual ~Exception() = default;

         virtual const char* what() const throw() { return cause.c_str(); }

      private:
         std::string cause;
      };

      /** Simple mechanism to facilitate logging throughout the application */
      class Log
      {
      public:
         enum Level
         {
            Error,
            Warn,
            Info,
            Debug,
            Trace
         };

         ~Log();

         template<typename T> Log& operator <<(const T &p)
         {
            if (not domLogged)
            {
               ss << std::left << std::setw(12);
               domLogged = true;
            }

            ss << p;
            return *this;
         }

         static void init(const std::string &file);

      private:
         bool domLogged = false;
         std::stringstream ss;
         static std::ostream *logStream;
      };
   }
}

// TODO: Replace this with an actual log library
#define LOG_LEVEL qcore::util::Log::Info

#define LOG(level) if (level <= LOG_LEVEL) qcore::util::Log()

#define LOG_INIT(file) qcore::util::Log::init(file)
#define LOG_TRACE(DOM) LOG(qcore::util::Log::Trace) << DOM << " [trace] "
#define LOG_DEBUG(DOM) LOG(qcore::util::Log::Debug) << DOM << " [debug] "
#define LOG_INFO(DOM)  LOG(qcore::util::Log::Info)  << DOM << " [info]  "
#define LOG_WARN(DOM)  LOG(qcore::util::Log::Warn)  << DOM << " [warn]  "
#define LOG_ERROR(DOM) LOG(qcore::util::Log::Error) << DOM << " [error] "


#endif // Header_qcore_Util
