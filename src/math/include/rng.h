#pragma once
#include <array>
#include <chrono>
#include <random>
#include <string>

namespace ntru {

  // ������� RNG (�� �������������)
  struct SimpleRng {
    using result_type = uint32_t;
    std::mt19937_64 eng;
    SimpleRng() {
      uint64_t s = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
      std::random_device rd;
      s ^= (static_cast<uint64_t>(rd()) << 1);
      eng.seed(s);
    }
    static constexpr result_type min() { return 0; }
    static constexpr result_type max() { return 0xFFFFFFFFu; }
    result_type operator()() { return static_cast<result_type>(eng() & 0xFFFFFFFFu); }
    void mix_user_entropy(const std::string &extra) {
      std::seed_seq seq(extra.begin(), extra.end());
      std::array<uint32_t, 4> tmp{};
      seq.generate(tmp.begin(), tmp.end());
      const uint64_t s =
          (static_cast<uint64_t>(tmp[0]) << 32) ^ tmp[1] ^ (static_cast<uint64_t>(tmp[2]) << 16) ^ tmp[3];
      eng.seed(eng() ^ s);
    }
  };

  // ������� ���������� ���������� (CLT)
  [[nodiscard]] intple_gauss_int(SimpleRng &rng, double sigma);

} // namespace ntru
