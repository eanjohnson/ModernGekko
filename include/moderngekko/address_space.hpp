#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace moderngekko
{
namespace address_space_detail
{
inline std::uint32_t ToPhysicalAddress(std::uint32_t address)
{
  if ((address & 0x80000000u) != 0u)
    return address & 0x3FFFFFFFu;
  return address;
}

template <typename Container>
auto ResolveRegion(Container& memory, std::uint32_t offset, std::size_t size)
    -> decltype(memory.data())
{
  if (offset > memory.size() || size > memory.size() - offset)
    return nullptr;
  return memory.data() + offset;
}
}

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

  /* Defined here rather than out of line because the vertex loader calls it
   * once per attribute per vertex -- four or five times for each of the 14 M
   * vertices in the slow window -- and across a translation unit boundary the
   * compiler could see neither the two span sizes nor that the whole thing is
   * a pair of compares. What it computes is unchanged. */
  std::uint8_t* Resolve(std::uint32_t address, std::size_t size)
  {
    const std::uint32_t physical = address_space_detail::ToPhysicalAddress(address);
    if (physical < m_mem1.size())
      return address_space_detail::ResolveRegion(m_mem1, physical, size);
    if (physical >= Mem2Base)
      return address_space_detail::ResolveRegion(m_mem2, physical - Mem2Base, size);
    return nullptr;
  }

  const std::uint8_t* Resolve(std::uint32_t address, std::size_t size) const
  {
    const std::uint32_t physical = address_space_detail::ToPhysicalAddress(address);
    if (physical < m_mem1.size())
      return address_space_detail::ResolveRegion(m_mem1, physical, size);
    if (physical >= Mem2Base)
      return address_space_detail::ResolveRegion(m_mem2, physical - Mem2Base, size);
    return nullptr;
  }

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
