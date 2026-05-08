#include "wrapping_integers.hh"
#include "debug.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  // return Wrap32 { 0 };
  return zero_point + static_cast<uint32_t>(n);
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // debug( "unimplemented unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  // return {};
  auto offset = raw_value_ - zero_point.raw_value_;
  const uint64_t MOD = 1ULL << 32;
  uint64_t k = checkpoint / MOD;
  auto best = offset + MOD * k;
  uint64_t best_gap = best > checkpoint ? best - checkpoint : checkpoint - best;
  if (k > 0){
    uint64_t prev = offset + (k - 1) * MOD;
    auto gap = prev > checkpoint ? prev - checkpoint : checkpoint - prev;
    if (gap < best_gap) {
      best_gap = gap;
      best = prev;
    }
  }
  auto next_gap = offset + (k + 1) * MOD > checkpoint ? offset + (k + 1) * MOD - checkpoint : checkpoint - (offset + (k + 1) * MOD);
  if (next_gap < best_gap) {best = offset + (k + 1) * MOD;}
  return best;


}
