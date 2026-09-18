#pragma once

/* An optional profiling hook for an embedder that samples from another thread.
 *
 * The GPU stack has no idea what an embedder's phases are and must keep having
 * none, so it is handed a byte to write and the value to write there. Both are
 * null/zero until someone sets them, which is what makes this free in a build
 * that is not being profiled: one predictable branch per scope.
 *
 * Nothing here is thread-safe by design. The slot is written by the thread
 * running the GPU stack and read by a sampler that tolerates a torn value --
 * a sampler that lands mid-store attributes one sample to the wrong phase and
 * that is the whole cost of being wrong here. */

namespace moderngekko
{
extern volatile unsigned char *g_phase_slot;
extern unsigned char g_phase_vertex_load;

/* Scoped, because Decode has several early returns. */
struct PhaseScope
{
  volatile unsigned char *slot;
  unsigned char was;
  explicit PhaseScope(unsigned char phase) : slot(g_phase_slot), was(0)
  {
    if (slot != nullptr)
    {
      was = *slot;
      *slot = phase;
    }
  }
  ~PhaseScope()
  {
    if (slot != nullptr)
      *slot = was;
  }
  PhaseScope(const PhaseScope &) = delete;
  PhaseScope &operator=(const PhaseScope &) = delete;
};
}
