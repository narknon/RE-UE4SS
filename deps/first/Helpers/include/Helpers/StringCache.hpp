#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <concepts>
#include <list>
#include <Helpers/String.hpp>

namespace RC {

/**
 * High-performance thread-local string cache for frequent string conversions.
 * 
 * This cache stores UTF-8 conversions of frequently-used strings, avoiding
 * repeated conversions of the same content. Particularly useful for:
 * - ImGui labels that change dynamically
 * - Localization lookups
 * - Any repeated string type conversions
 * 
 * Thread Safety: Each thread has its own cache instance (thread_local).
 * The cache itself is thread-safe, but users must ensure source strings
 * are not modified by other threads during conversion.
 */
class StringCache {
private:
    // A generic, bounded LRU cache structure.
    template<typename ViewType>
    struct Cache {
        // The key is the string content itself. The map makes a copy for ownership.
        std::unordered_map<ViewType, std::string> map;
        std::list<ViewType> lru_queue;
        static constexpr size_t MAX_SIZE = 256; // A reasonable default limit
    };

    // One cache instance for each source character type.
    Cache<std::string_view>  m_char_cache;
    Cache<std::wstring_view> m_wchar_cache;
    Cache<std::u16string_view> m_u16_cache;
    Cache<std::u32string_view> m_u32_cache;
    Cache<std::u8string_view> m_u8_cache;

    // Generic implementation for getting/setting the cached value.
    template<typename ViewType>
    const char* get_cached_utf8(ViewType source_view, Cache<ViewType>& cache) {
        auto it = cache.map.find(source_view);

        if (it != cache.map.end()) {
            // Found: Move to front of LRU queue to mark it as recently used.
            // We find the iterator in the list and splice it to the front.
            auto lru_it = std::find(cache.lru_queue.begin(), cache.lru_queue.end(), it->first);
            if (lru_it != cache.lru_queue.begin()) {
                cache.lru_queue.splice(cache.lru_queue.begin(), cache.lru_queue, lru_it);
            }
            return it->second.c_str();
        } else {
            // Not found: Evict if cache is full.
            if (cache.map.size() >= Cache<ViewType>::MAX_SIZE) {
                ViewType oldest_key = cache.lru_queue.back();
                cache.lru_queue.pop_back();
                cache.map.erase(oldest_key);
            }

            // Convert and cache the new string.
            std::string utf8_string = to_utf8_string(source_view);
            const char* c_str = utf8_string.c_str();
            
            // Emplace the view, which creates a key copy, then move the value.
            auto [new_it, _] = cache.map.emplace(source_view, std::move(utf8_string));
            
            // Store the key from the map (which is stable) in the LRU queue.
            cache.lru_queue.push_front(new_it->first);
            
            return c_str;
        }
    }

public:
    /**
     * Convert any string-like type to UTF-8, with caching for non-char types.
     * 
     * @param str The string to convert
     * @return Pointer to UTF-8 string (valid until cache eviction or thread exit)
     */
    template<StringLike T>
    const char* convert(T&& str) {
        // Create a view to work with. This is a no-op for views/pointers.
        auto source_view = std::basic_string_view(str);
        
        // Dispatch to the appropriate internal cache based on the character type.
        using SourceCharT = typename decltype(source_view)::value_type;
        if constexpr (std::is_same_v<SourceCharT, char>) {
            return get_cached_utf8(source_view, m_char_cache);
        } else if constexpr (std::is_same_v<SourceCharT, wchar_t>) {
            return get_cached_utf8(source_view, m_wchar_cache);
        } else if constexpr (std::is_same_v<SourceCharT, char16_t>) {
            return get_cached_utf8(source_view, m_u16_cache);
        } else if constexpr (std::is_same_v<SourceCharT, char32_t>) {
            return get_cached_utf8(source_view, m_u32_cache);
        } else if constexpr (std::is_same_v<SourceCharT, char8_t>) {
            return get_cached_utf8(source_view, m_u8_cache);
        }
    }
    
    void clear() {
        m_char_cache.map.clear();
        m_char_cache.lru_queue.clear();
        m_wchar_cache.map.clear();
        m_wchar_cache.lru_queue.clear();
        m_u16_cache.map.clear();
        m_u16_cache.lru_queue.clear();
        m_u32_cache.map.clear();
        m_u32_cache.lru_queue.clear();
        m_u8_cache.map.clear();
        m_u8_cache.lru_queue.clear();
    }
    
    size_t size() const {
        return m_char_cache.map.size() + m_wchar_cache.map.size() + m_u16_cache.map.size() + m_u32_cache.map.size() + m_u8_cache.map.size();
    }
};

/**
 * Get the thread-local string cache instance.
 * Each thread has its own cache to avoid synchronization overhead.
 */
inline StringCache& GetThreadLocalStringCache() {
    thread_local StringCache instance;
    return instance;
}

} // namespace RC