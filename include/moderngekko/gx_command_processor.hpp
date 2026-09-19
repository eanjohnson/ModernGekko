#pragma once

#include "moderngekko/gpu_backend.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace moderngekko
{
class AddressSpace;

class GxCommandProcessor final
{
public:
  explicit GxCommandProcessor(GpuBackend* backend = nullptr, AddressSpace* memory = nullptr);

  void SetBackend(GpuBackend* backend);
  void SetAddressSpace(AddressSpace* memory);
  void Reset();
  void Write(std::uint64_t value, std::uint8_t size);
  void WriteBytes(std::span<const std::uint8_t> bytes);

  std::size_t GetBufferedByteCount() const;
  std::uint64_t GetCommandCount() const;
  std::uint64_t GetUnknownOpcodeCount() const;
  std::uint32_t GetVertexSize(std::uint8_t vat) const;

  /* The per-VAT vertex size is a pure function of VCD low/high and that
   * VAT's three words, and all five only change through LoadCp -- so it is
   * memoised rather than recomputed on every draw command. Off by the knob
   * so one build can measure both sides. */
  void SetVertexSizeCache(bool enabled);
  std::uint64_t GetVertexSizeHits() const;
  std::uint64_t GetVertexSizeMisses() const;

private:
  /* `need`, when given, comes back as the number of bytes this command will
   * take once the rest of it arrives -- so a caller feeding the FIFO four
   * bytes at a time can wait instead of re-parsing a half-written command on
   * every write. It is an out-parameter rather than a member because
   * ExecuteDisplayList decodes a second stream from inside this one. */
  std::size_t DecodeOne(std::span<const std::uint8_t> data,
                        std::size_t* need = nullptr);
  std::uint32_t ComputeVertexSize(std::uint8_t vat) const;
  void ExecuteDisplayList(std::uint32_t address, std::uint32_t size);
  void LoadCp(std::uint8_t command, std::uint32_t value);

  GpuBackend* m_backend = nullptr;
  AddressSpace* m_memory = nullptr;
  std::vector<std::uint8_t> m_buffer;
  /* How many bytes the command at the head of m_buffer needs, or 0 when that
   * is not known yet. */
  std::size_t m_pending_need = 0;
  std::uint32_t m_vcd_low = 0;
  std::uint32_t m_vcd_high = 0;
  std::array<std::array<std::uint32_t, 3>, 8> m_vat{};
  std::array<std::uint32_t, 16> m_array_bases{};
  std::array<std::uint8_t, 16> m_array_strides{};
  mutable std::array<std::uint32_t, 8> m_vertex_size{};
  mutable std::uint8_t m_vertex_size_valid = 0;
  mutable std::uint64_t m_vertex_size_hits = 0;
  mutable std::uint64_t m_vertex_size_misses = 0;
  bool m_vertex_size_cache = true;
  std::uint64_t m_command_count = 0;
  std::uint64_t m_unknown_opcode_count = 0;
  std::uint8_t m_display_list_depth = 0;
};
}
