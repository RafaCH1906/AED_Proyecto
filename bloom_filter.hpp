#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>


struct Probe {
    std::size_t hash_id;
    std::uint64_t raw;
    std::size_t index;
    bool bit_before;
};

class BloomFilter {
public:

    BloomFilter(std::size_t m_bits, std::size_t k_hashes);



    static BloomFilter from_capacity(std::size_t n_expected, double p_target);


    void insert(const std::string& key, std::vector<Probe>* probes = nullptr);


    bool contains(const std::string& key, std::vector<Probe>* probes = nullptr) const;

    void clear();

    std::size_t m() const { return m_bits_; }
    std::size_t k() const { return k_hashes_; }
    std::size_t inserted() const { return n_inserted_; }
    std::size_t bits_set() const { return bits_set_; }
    bool bit(std::size_t i) const;
    std::vector<int> snapshot() const;
    double load_factor() const;
    double false_positive_rate() const;

private:

    static std::uint64_t base_hash(const std::string& key, std::uint64_t seed);
    static std::uint64_t mix64(std::uint64_t z);
    std::uint64_t nth_hash(std::size_t i, std::uint64_t h1, std::uint64_t h2) const;

    std::size_t m_bits_;
    std::size_t k_hashes_;
    std::size_t n_inserted_;
    std::size_t bits_set_;
    std::vector<std::uint64_t> words_; 
};
