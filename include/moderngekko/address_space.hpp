#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace moderngekko
{
class AddressSpace final
{
public:
  static constexpr std::uint32_t Mem1Base = 0x00000000u;
  static constexpr std::uint32_t Mem2Base = 0x10000000u;
  static constexpr std::uint32_t RetailMem1Size = 0x01800000u;
  static constexpr std::uint32_t RetailMem2Size = 0x04000000u;

  explicit AddressSpace(bool enable_mem2 = false);
  AddressSpace(std::size_t mem1_size, std::size_t mem2_size);
  /* Adopts memory owned by the caller. Needed where the guest RAM cannot come
   * from the heap -- the PS Vita port maps its 24 MB MEM1 and 64 MB MEM2 with
   * sceKernelAllocMemBlock, and routing them through a std::vector would move
   * 88 MB onto the newlib heap. The spans must outlive the AddressSpace. */
  AddressSpace(std::span<std::uint8_t> mem1, std::span<std::uint8_t> mem2);

  std::span<std::uint8_t> GetMem1();
  std::span<const std::uint8_t> GetMem1() const;
  std::span<std::uint8_t> GetMem2();
  std::span<const std::uint8_t> GetMem2() const;

  std::uint8_t* Resolve(std::uint32_t address, std::size_t size);
  const std::uint8_t* Resolve(std::uint32_t address, std::size_t size) const;

  bool Read8(std::uint32_t address, std::uint8_t* value) const;
  bool Read16(std::uint32_t address, std::uint16_t* value) const;
  bool Read32(std::uint32_t address, std::uint32_t* value) const;
  bool Read64(std::uint32_t address, std::uint64_t* value) const;

  bool Write8(std::uint32_t address, std::uint8_t value);
  bool Write16(std::uint32_t address, std::uint16_t value);
  bool Write32(std::uint32_t address, std::uint32_t value);
  bool Write64(std::uint32_t address, std::uint64_t value);

  void Clear();

private:
  /* Populated only when this AddressSpace owns its memory; the spans below are
   * what every access goes through, so both modes share one code path. */
  std::vector<std::uint8_t> m_owned_mem1;
  std::vector<std::uint8_t> m_owned_mem2;
  std::span<std::uint8_t> m_mem1;
  std::span<std::uint8_t> m_mem2;
};
}
