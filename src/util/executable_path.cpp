/**
 * Implémentation du chemin absolu de l'exécutable (Windows, Linux, macOS).
 */

#include "executable_path.hpp"
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#else
#include <unistd.h>
#include <limits.h>
#endif

namespace taskman {

std::string get_executable_path() {
#if defined(_WIN32) || defined(_WIN64)
    // MAX_PATH (260) is often too small; use a larger buffer (Windows supports up to 32767).
    std::vector<wchar_t> buf(32768, 0);
    DWORD n = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
    if (n == 0 || n >= buf.size()) return {};
    try {
        std::filesystem::path p(buf.data());
        return p.lexically_normal().string();
    } catch (...) {
        return {};
    }
#elif defined(__APPLE__)
    char buf[PATH_MAX];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) != 0) return {};
    try {
        return std::filesystem::absolute(buf).string();
    } catch (...) {
        return buf;
    }
#else
    char buf[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) return {};
    buf[n] = '\0';
    try {
        return std::filesystem::absolute(buf).string();
    } catch (...) {
        return buf;
    }
#endif
}

} // namespace taskman
