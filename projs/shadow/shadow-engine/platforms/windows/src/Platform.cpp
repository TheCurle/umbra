#include <shadow/platform/Common.h>

#define NOGDI
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
#include <psapi.h>
#include "spdlog/spdlog.h"

namespace Platform {

  bool GetCommandLine(const std::string& data) {
      const char* cli = ::GetCommandLine();
      ((std::string&)data).append(cli);
      return true;
  }

  void DumpSystemData() {
      DWORD version = ::GetVersion();
      DWORD major = (DWORD)(LOBYTE(LOWORD(version)));
      DWORD minor = (DWORD)(HIBYTE(LOWORD(version)));

      DWORD build = 0;

      if (version < 0x80000000)
          build = (DWORD(HIWORD(version)));

      spdlog::info("OS version: ", size_t(major), ".", size_t(minor), " (", size_t(build), ")");

      SYSTEM_INFO sys;
      ::GetSystemInfo(&sys);
      spdlog::info("Page size: ", size_t(sys.dwPageSize));
      spdlog::info("Processors: ", size_t(sys.dwNumberOfProcessors));
      spdlog::info("Allocation gr.: ", size_t(sys.dwAllocationGranularity));
  }

  void GetExecutableDirectory(const std::string& data) {
      char tmp[MAX_PATH];
      if (::GetCurrentDirectory(lengthOf(tmp), tmp) == ERROR_SUCCESS) {
          ((std::string&)data).append(tmp);
      }
  }

  size_t GetProcessMemory() {
      PROCESS_MEMORY_COUNTERS counters;
      if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) != 0) {
          return counters.WorkingSetSize;
      }

      assert(false);
      return 0;
  }

}