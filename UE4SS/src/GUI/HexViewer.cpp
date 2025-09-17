#include <GUI/HexViewer.hpp>
#include <PatternSleuthBindings.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome5.h>

namespace RC::GUI
{
    HexViewer::HexViewer()
    {
        m_data_buffer.resize(m_bytes_per_row * m_rows_to_display);
    }

    void HexViewer::Open(uint64_t address)
    {
        m_is_open = true;
        m_current_address = address & ~0xF; // Align to 16 bytes
        m_goto_address = m_current_address;
        ReadMemoryRegion();
    }

    void HexViewer::Render()
    {
        if (!m_is_open) return;

        ImGui::SetNextWindowSize(ImVec2(750, 550), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Memory Viewer", &m_is_open))
        {
            // Navigation controls
            if (ImGui::Button(ICON_FA_ARROW_UP))
            {
                m_current_address = std::max<uint64_t>(m_current_address - (m_bytes_per_row * 8), 0);
                ReadMemoryRegion();
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROW_DOWN))
            {
                m_current_address += m_bytes_per_row * 8;
                ReadMemoryRegion();
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROW_LEFT))
            {
                m_current_address = std::max<uint64_t>(m_current_address - (m_bytes_per_row * m_rows_to_display), 0);
                ReadMemoryRegion();
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ARROW_RIGHT))
            {
                m_current_address += m_bytes_per_row * m_rows_to_display;
                ReadMemoryRegion();
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(150);
            ImGui::InputScalar("Address", ImGuiDataType_U64, &m_goto_address, nullptr, nullptr, "%016llX", ImGuiInputTextFlags_CharsHexadecimal);

            ImGui::SameLine();
            if (ImGui::Button("Go") || ImGui::IsKeyPressed(ImGuiKey_Enter))
            {
                m_current_address = m_goto_address & ~0xF; // Align to 16 bytes
                ReadMemoryRegion();
            }

            ImGui::Separator();

            // Main hex view
            RenderHexView();
        }
        ImGui::End();
    }

    void HexViewer::ReadMemoryRegion()
    {
        if (!PatternSleuthBindings::ReadMemory(m_current_address, m_data_buffer.data(), m_data_buffer.size()))
        {
            // Clear buffer if read failed
            std::fill(m_data_buffer.begin(), m_data_buffer.end(), 0);
        }
    }

    void HexViewer::RenderHexView()
    {
        ImGui::BeginChild("HexData", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove);

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font (should be monospace for hex)
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(3, 2));

        // Create table
        if (ImGui::BeginTable("HexTable", 18, ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter))
        {
            // Setup columns
            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 140.0f);
            for (int i = 0; i < 16; i++)
            {
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 25.0f);
            }
            ImGui::TableSetupColumn("ASCII", ImGuiTableColumnFlags_WidthFixed, 140.0f);

            // Header
            ImGui::TableHeadersRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Address");
            for (int i = 0; i < 16; i++)
            {
                ImGui::TableSetColumnIndex(i + 1);
                ImGui::Text("%X", i);
            }
            ImGui::TableSetColumnIndex(17);
            ImGui::Text("ASCII");

            // Data rows
            for (size_t row = 0; row < m_rows_to_display; ++row)
            {
                ImGui::TableNextRow();

                uint64_t row_address = m_current_address + (row * m_bytes_per_row);

                // Address column
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%016llX", row_address);

                // Hex bytes
                for (size_t col = 0; col < m_bytes_per_row; ++col)
                {
                    ImGui::TableSetColumnIndex(col + 1);
                    size_t idx = row * m_bytes_per_row + col;
                    ImGui::Text("%02X", m_data_buffer[idx]);
                }

                // ASCII column
                ImGui::TableSetColumnIndex(17);
                std::string ascii_str;
                for (size_t col = 0; col < m_bytes_per_row; ++col)
                {
                    size_t idx = row * m_bytes_per_row + col;
                    uint8_t byte = m_data_buffer[idx];
                    ascii_str += (byte >= 32 && byte < 127) ? static_cast<char>(byte) : '.';
                }
                ImGui::Text("%s", ascii_str.c_str());
            }

            ImGui::EndTable();
        }

        ImGui::PopStyleVar();
        ImGui::PopFont();

        ImGui::EndChild();
    }
} // namespace RC::GUI