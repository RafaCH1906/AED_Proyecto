// bloom_filter.hpp
// Implementación propia de un Bloom Filter (sin librerías de terceros).
// CS2023 - Algoritmos y Estructuras de Datos - UTEC 2026-2
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Resultado de una sondeo (probe) de una de las k funciones hash.
struct Probe {
    std::size_t hash_id;   // i-esima funcion hash (0..k-1)
    std::uint64_t raw;     // valor hash antes del modulo
    std::size_t index;     // posicion en el vector de bits
    bool bit_before;       // valor del bit ANTES de la operacion
};

class BloomFilter {
public:
    // Constructor directo: m bits, k funciones hash.
    BloomFilter(std::size_t m_bits, std::size_t k_hashes);

    // Constructor optimo: n elementos esperados y probabilidad p de falso positivo.
    //   m = ceil(-n * ln(p) / (ln 2)^2)    k = round((m/n) * ln 2)
    static BloomFilter from_capacity(std::size_t n_expected, double p_target);

    // Inserta una clave. Si probes != nullptr guarda el detalle de los k sondeos.
    void insert(const std::string& key, std::vector<Probe>* probes = nullptr);

    // Consulta de pertenencia. false => definitivamente NO esta.
    //                          true  => probablemente SI esta (puede ser falso positivo).
    bool contains(const std::string& key, std::vector<Probe>* probes = nullptr) const;

    void clear();

    // --- Observadores ---
    std::size_t m() const { return m_bits_; }
    std::size_t k() const { return k_hashes_; }
    std::size_t inserted() const { return n_inserted_; }
    std::size_t bits_set() const { return bits_set_; }
    bool bit(std::size_t i) const;
    std::vector<int> snapshot() const;          // vector de 0/1 para la animacion
    double load_factor() const;                 // bits en 1 / m
    double false_positive_rate() const;         // (1 - e^(-k n / m))^k

private:
    // Kirsch-Mitzenmacher: h_i(x) = h1(x) + i*h2(x) + i^2  (mod m)
    // h1, h2 derivados de una funcion hash de 64 bits propia (mezcla tipo FNV + avalancha).
    static std::uint64_t base_hash(const std::string& key, std::uint64_t seed);
    static std::uint64_t mix64(std::uint64_t z);
    std::uint64_t nth_hash(std::size_t i, std::uint64_t h1, std::uint64_t h2) const;

    std::size_t m_bits_;
    std::size_t k_hashes_;
    std::size_t n_inserted_;
    std::size_t bits_set_;
    std::vector<std::uint64_t> words_;  // vector de bits empaquetado (64 bits por palabra)
};
