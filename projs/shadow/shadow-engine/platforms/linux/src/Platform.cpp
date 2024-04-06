#include <shadow/platform/Common.h>
#include <string>

namespace Platform {

  bool GetCommandLine(const std::string& data) {
      ((std::string&)data).append("Unable to get command line data on Linux.");
      return true;
  }

  void DumpSystemData() {

      struct utsname tmp;
      if (uname(&tmp) == 0) {
          spdlog::info("System name: ", tmp.sysname);
          spdlog::info("Node name: ", tmp.nodename);
          spdlog::info("OS release: ", tmp.release);
          spdlog::info("OS version: ", tmp.version);
          spdlog::info("Computer type: ", tmp.machine);
      } else {
          spdlog::warning("Cannot get system data.");
      }
  }


  void GetExecutableDirectory(const std::string& data) {
      if (!getcwd(((std::string&)data).data(), 255))
          ((std::string&)data).data()[0] = 0;
  }

}