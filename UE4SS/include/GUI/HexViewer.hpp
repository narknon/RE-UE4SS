#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace RC::GUI
{
    class HexViewer
    {
    public:
        HexViewer();
        ~HexViewer() = default;

        // Open hex viewer at address
        void Open(uint64_t address);

        // Render the hex viewer window
        void Render();

        // Check if window is open
        bool IsOpen() const { return m_is_open; }

    private:
        bool m_is_open{false};
        uint64_t m_current_address{0};
        uint64_t m_goto_address{0};
        std::vector<uint8_t> m_data_buffer;
        static constexpr size_t m_bytes_per_row{16};
        static constexpr size_t m_rows_to_display{32};

        void ReadMemoryRegion();
        void RenderHexView();
    };
} // namespace RC::GUI