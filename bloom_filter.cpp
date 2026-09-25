// bloom_filter.cpp
#include "bloom_filter.hpp"

#include <cmath>
#include <stdexcept>

// ---------------------------------------------------------------- construccion

BloomFilter::BloomFilter(std::size_t m_bits, std::size_t k_hashes)
    : m_bits_(m_bits),
      k_hashes_(k_hashes),
      n_inserted_(0),
      bits_set_(0),
      words_((m_bits + 63) / 64, 0ULL) {
    if (m_bits == 0) throw std::invalid_argument("m debe ser > 0");
    if (k_hashes == 0) throw std::invalid_argument("k debe ser > 0");
}

BloomFilter BloomFilter::from_capacity(std::size_t n_expected, double p_target) {
    if (n_expected == 0) throw std::invalid_argument("n debe ser > 0");
    if (!(p_target > 0.0 && p_target < 1.0)) throw std::invalid_argument("p en (0,1)");

    const double ln2 = std::log(2.0);
    double m = -static_cast<double>(n_expected) * std::log(p_target) / (ln2 * ln2);
    std::size_t m_bits = static_cast<std::size_t>(std::ceil(m));
    std::size_t k = static_cast<std::size_t>(
        std::llround((static_cast<double>(m_bits) / n_expected) * ln2));
    if (k == 0) k = 1;
    return BloomFilter(m_bits, k);
}

// ---------------------------------------------------------------------- hashing

// Avalancha de 64 bits (finalizer estilo splitmix64): dispersa los bits altos.
std::uint64_t BloomFilter::mix64(std::uint64_t z) {
    z += 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

// Hash base de la cadena: FNV-1a de 64 bits sembrado + avalancha final.
std::uint64_t BloomFilter::base_hash(const std::string& key, std::uint64_t seed) {
    std::uint64_t h = 0xCBF29CE484222325ULL ^ seed;
    for (unsigned char c : key) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 0x100000001B3ULL;
    }
    return mix64(h);
}

// Kirsch-Mitzenmacher (doble hashing mejorado): genera k hashes con solo 2 hashes base.
std::uint64_t BloomFilter::nth_hash(std::size_t i, std::uint64_t h1, std::uint64_t h2) const {
    return h1 + static_cast<std::uint64_t>(i) * h2 +
           static_cast<std::uint64_t>(i) * static_cast<std::uint64_t>(i);
}

// ------------------------------------------------------------------ operaciones

void BloomFilter::insert(const std::string& key, std::vector<Probe>* probes) {
    const std::uint64_t h1 = base_hash(key, 0x51ED270BULL);
    const std::uint64_t h2 = base_hash(key, 0x2545F491ULL) | 1ULL;  // impar => mejor dispersion

    for (std::size_t i = 0; i < k_hashes_; ++i) {
        const std::uint64_t raw = nth_hash(i, h1, h2);
        const std::size_t idx = static_cast<std::size_t>(raw % m_bits_);
        const bool before = bit(idx);

        if (!before) {
            words_[idx >> 6] |= (1ULL << (idx & 63));
            ++bits_set_;
        }
        if (probes) probes->push_back(Probe{i, raw, idx, before});
    }
    ++n_inserted_;
}

bool BloomFilter::contains(const std::string& key, std::vector<Probe>* probes) const {
    const std::uint64_t h1 = base_hash(key, 0x51ED270BULL);
    const std::uint64_t h2 = base_hash(key, 0x2545F491ULL) | 1ULL;

    bool present = true;
    for (std::size_t i = 0; i < k_hashes_; ++i) {
        const std::uint64_t raw = nth_hash(i, h1, h2);
        const std::size_t idx = static_cast<std::size_t>(raw % m_bits_);
        const bool b = bit(idx);

        if (probes) probes->push_back(Probe{i, raw, idx, b});

        if (!b) {
            present = false;
            // Corte temprano solo si no estamos trazando (la animacion muestra los k sondeos).
            if (!probes) return false;
        }
    }
    return present;
}

void BloomFilter::clear() {
    for (auto& w : words_) w = 0ULL;
    n_inserted_ = 0;
    bits_set_ = 0;
}

// ------------------------------------------------------------------ observadores

bool BloomFilter::bit(std::size_t i) const {
    return (words_[i >> 6] >> (i & 63)) & 1ULL;
}

std::vector<int> BloomFilter::snapshot() const {
    std::vector<int> out(m_bits_);
    for (std::size_t i = 0; i < m_bits_; ++i) out[i] = bit(i) ? 1 : 0;
    return out;
}

double BloomFilter::load_factor() const {
    return static_cast<double>(bits_set_) / static_cast<double>(m_bits_);
}

double BloomFilter::false_positive_rate() const {
    const double kn_m = -static_cast<double>(k_hashes_) *
                        static_cast<double>(n_inserted_) / static_cast<double>(m_bits_);
    return std::pow(1.0 - std::exp(kn_m), static_cast<double>(k_hashes_));
}
