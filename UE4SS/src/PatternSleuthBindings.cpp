#include <PatternSleuthBindings.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include <UE4SSProgram.hpp>
#include <cstring>

// External C functions from patternsleuth_bind
extern "C" {
    bool ps_scan_pattern(const char* pattern_str, uint64_t** results, size_t* count);
    bool ps_scan_string(const char* search_str, uint64_t** results, size_t* count);
    bool ps_scan_wstring(const uint16_t* search_str, uint64_t** results, size_t* count);
    bool ps_scan_xref(uint64_t target_address, uint64_t** results, size_t* count);
    void ps_free_results(uint64_t* results, size_t count);
    uint64_t ps_resolve_single(const char* resolver_name);
    size_t ps_resolve_batch(const char** resolver_names, uint64_t* results);
    bool ps_get_resolver_names(const char** names, size_t* count);
    bool ps_resolver_exists(const char* resolver_name);
    uint64_t ps_get_vtable_size(uint64_t vtable_address);
}

namespace RC
{
    bool PatternSleuthBindings::s_initialized = false;

    bool PatternSleuthBindings::Initialize()
    {
        if (s_initialized) return true;

        // PatternSleuth doesn't need explicit initialization for internal process
        s_initialized = true;
        Output::send(STR("PatternSleuth bindings initialized\n"));
        return true;
    }

    uint64_t PatternSleuthBindings::ResolveByName(const std::string& resolver_name)
    {
        if (!s_initialized && !Initialize()) return 0;

        return ps_resolve_single(resolver_name.c_str());
    }

    std::vector<uint64_t> PatternSleuthBindings::ResolveBatch(const std::vector<std::string>& resolver_names)
    {
        if (!s_initialized && !Initialize()) return {};

        // Create null-terminated array of C strings
        std::vector<const char*> c_names;
        for (const auto& name : resolver_names)
        {
            c_names.push_back(name.c_str());
        }
        c_names.push_back(nullptr); // null terminator

        std::vector<uint64_t> results(resolver_names.size(), 0);
        size_t count = ps_resolve_batch(c_names.data(), results.data());

        results.resize(count);
        return results;
    }

    std::vector<std::string> PatternSleuthBindings::GetResolverNames()
    {
        if (!s_initialized && !Initialize()) return {};

        const char* names[256]; // Should be enough for all resolvers
        size_t count = 0;

        if (!ps_get_resolver_names(names, &count))
        {
            return {};
        }

        std::vector<std::string> result;
        result.reserve(count);

        for (size_t i = 0; i < count; ++i)
        {
            if (names[i])
            {
                result.emplace_back(names[i]);
            }
        }

        return result;
    }

    std::vector<uint64_t> PatternSleuthBindings::ScanPattern(const std::string& pattern)
    {
        if (!s_initialized && !Initialize()) return {};

        uint64_t* raw_results = nullptr;
        size_t count = 0;

        if (!ps_scan_pattern(pattern.c_str(), &raw_results, &count))
        {
            return {};
        }

        std::vector<uint64_t> results;
        if (count > 0 && raw_results)
        {
            results.assign(raw_results, raw_results + count);
            ps_free_results(raw_results, count);
        }

        return results;
    }

    std::vector<uint64_t> PatternSleuthBindings::ScanString(const std::string& str)
    {
        if (!s_initialized && !Initialize()) return {};

        uint64_t* raw_results = nullptr;
        size_t count = 0;

        if (!ps_scan_string(str.c_str(), &raw_results, &count))
        {
            return {};
        }

        std::vector<uint64_t> results;
        if (count > 0 && raw_results)
        {
            results.assign(raw_results, raw_results + count);
            ps_free_results(raw_results, count);
        }

        return results;
    }

    std::vector<uint64_t> PatternSleuthBindings::ScanWString(const std::wstring& str)
    {
        if (!s_initialized && !Initialize()) return {};

        uint64_t* raw_results = nullptr;
        size_t count = 0;

        // Convert wstring to uint16_t array for patternsleuth
        std::vector<uint16_t> u16str;
        for (wchar_t wc : str)
        {
            u16str.push_back(static_cast<uint16_t>(wc));
        }
        u16str.push_back(0); // null terminator

        if (!ps_scan_wstring(u16str.data(), &raw_results, &count))
        {
            return {};
        }

        std::vector<uint64_t> results;
        if (count > 0 && raw_results)
        {
            results.assign(raw_results, raw_results + count);
            ps_free_results(raw_results, count);
        }

        return results;
    }

    std::vector<uint64_t> PatternSleuthBindings::ScanXRef(uint64_t target_address)
    {
        if (!s_initialized && !Initialize()) return {};

        uint64_t* raw_results = nullptr;
        size_t count = 0;

        if (!ps_scan_xref(target_address, &raw_results, &count))
        {
            return {};
        }

        std::vector<uint64_t> results;
        if (count > 0 && raw_results)
        {
            results.assign(raw_results, raw_results + count);
            ps_free_results(raw_results, count);
        }

        return results;
    }

    uint64_t PatternSleuthBindings::GetVTableSize(uint64_t vtable_address)
    {
        if (!s_initialized && !Initialize()) return 0;

        return ps_get_vtable_size(vtable_address);
    }

    bool PatternSleuthBindings::ReadMemory(uint64_t address, void* buffer, size_t size)
    {
        if (!address || !buffer || !size) return false;

        try
        {
            // Direct memory read - this should work since we're in the same process
            std::memcpy(buffer, reinterpret_cast<const void*>(address), size);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
} // namespace RC