#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace RC
{
    class PatternSleuthBindings
    {
    public:
        // Initialize PatternSleuth with current process
        static bool Initialize();

        // Single resolver scan
        static uint64_t ResolveByName(const std::string& resolver_name);

        // Batch resolver scan
        static std::vector<uint64_t> ResolveBatch(const std::vector<std::string>& resolver_names);

        // Get available resolver names
        static std::vector<std::string> GetResolverNames();

        // Pattern scanning
        static std::vector<uint64_t> ScanPattern(const std::string& pattern);

        // String scanning
        static std::vector<uint64_t> ScanString(const std::string& str);
        static std::vector<uint64_t> ScanWString(const std::wstring& str);

        // Cross-reference scanning
        static std::vector<uint64_t> ScanXRef(uint64_t target_address);

        // VTable analysis
        static uint64_t GetVTableSize(uint64_t vtable_address);

        // Memory reading for hex viewer
        static bool ReadMemory(uint64_t address, void* buffer, size_t size);

    private:
        static bool s_initialized;
    };
} // namespace RC