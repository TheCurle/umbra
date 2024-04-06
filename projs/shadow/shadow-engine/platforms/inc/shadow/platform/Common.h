#pragma once

#ifdef __linux__
#define ifsystem(linux,windows,apple) linux
#elif __WIN32
#define ifsystem(linux,windows,apple) windows
#elif __APPLE__
#define ifsystem(linux,windows,apple) apple
#endif

// Clang on macOS has __GNUC__ defined to 4, for some reason.
#ifdef __clang__
#define ifcompiler(gcc,clang,msvc) clang
#elif __GNUC__
#define ifcompiler(gcc,clang,msvc) gcc
#elif _MSC_VER
#define ifcompiler(gcc,clang,msvc) msvc
#endif

#include <cstddef>
#include <string>
#include <cstdint>

template <typename T, uint32_t count> constexpr uint32_t lengthOf(const T (&)[count]) {
    return count;
};

namespace Platform {

  void DumpSystemData();

  bool GetCommandLine(const std::string& data);

  void GetExecutableDirectory(const std::string& data);

  size_t GetProcessMemory();



}