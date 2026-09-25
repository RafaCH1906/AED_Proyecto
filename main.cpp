// main.cpp
// Driver del proyecto: ejecuta el Bloom Filter REAL y emite la traza (trace.json)
// que consume la animacion. La animacion NO inventa pasos: solo dibuja esta traza.
#include "bloom_filter.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

// ------------------------------------------------------- utilidades JSON minimas

static std::string jstr(const std::string& s) {
    std::ostringstream o;
    o << '"';
    for (char c : s) {
        switch (c) {
            case '"':  o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\n': o << "\\n";  break;
            default:   o << c;
        }
    }
    o << '"';
    return o.str();
}

template <typename T>
static std::string jarr(const std::vector<T>& v) {
    std::ostringstream o;
    o << '[';
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) o << ',';
        o << v[i];
    }
    o << ']';
    return o.str();
}

static std::string jprobes(const std::vector<Probe>& ps) {
    std::ostringstream o;
    o << '[';
    for (std::size_t i = 0; i < ps.size(); ++i) {
        if (i) o << ',';
        o << "{\"hash_id\":" << ps[i].hash_id
          << ",\"raw\":" << ps[i].raw
          << ",\"index\":" << ps[i].index
          << ",\"bit_before\":" << (ps[i].bit_before ? "true" : "false") << '}';
    }
    o << ']';
    return o.str();
}

// ------------------------------------------------------------- escritor de traza

class Trace {
public:
    explicit Trace(const std::string& path) : out_(path) {
        out_ << std::setprecision(6) << std::fixed;
        out_ << "{\n  \"events\": [\n";
    }
    void event(const std::string& body) {
        if (n_++) out_ << ",\n";
        out_ << "    {" << body << "}";
    }
    void close(const std::string& meta) {
        out_ << "\n  ],\n  \"meta\": {" << meta << "}\n}\n";
        out_.close();
    }

private:
    std::ofstream out_;
    int n_ = 0;
};

// -------------------------------------------------------------- guion del video

int main() {
    // Parametros elegidos para que el vector de bits sea legible en pantalla: 4 x 16.
    const std::size_t M = 64;
    const std::size_t K = 3;
    BloomFilter bf(M, K);

    Trace tr("output/trace.json");

    std::ostringstream hdr;
    hdr << "\"type\":\"init\",\"m\":" << M << ",\"k\":" << K
        << ",\"bits\":" << jarr(bf.snapshot())
        << ",\"note\":" << jstr("Vector de bits vacio: m=64 bits, k=3 funciones hash");
    tr.event(hdr.str());

    auto emit_insert = [&](const std::string& key, const std::string& note) {
        std::vector<Probe> ps;
        bf.insert(key, &ps);
        std::ostringstream e;
        e << "\"type\":\"insert\",\"key\":" << jstr(key)
          << ",\"probes\":" << jprobes(ps)
          << ",\"bits\":" << jarr(bf.snapshot())
          << ",\"n\":" << bf.inserted()
          << ",\"bits_set\":" << bf.bits_set()
          << ",\"fpr\":" << bf.false_positive_rate()
          << ",\"note\":" << jstr(note);
        tr.event(e.str());
    };

    auto emit_query = [&](const std::string& key, bool really_in, const std::string& note) {
        std::vector<Probe> ps;
        const bool res = bf.contains(key, &ps);
        std::string verdict = !res ? "definitivamente_no"
                                   : (really_in ? "verdadero_positivo" : "FALSO_POSITIVO");
        std::ostringstream e;
        e << "\"type\":\"query\",\"key\":" << jstr(key)
          << ",\"probes\":" << jprobes(ps)
          << ",\"bits\":" << jarr(bf.snapshot())
          << ",\"result\":" << (res ? "true" : "false")
          << ",\"really_in\":" << (really_in ? "true" : "false")
          << ",\"verdict\":" << jstr(verdict)
          << ",\"note\":" << jstr(note);
        tr.event(e.str());
        return res;
    };

    // ---- 1) CASO BORDE: consulta sobre estructura vacia -------------------
    emit_query("manim", false,
               "Caso borde 1: filtro vacio. El primer bit en 0 basta para responder NO.");

    // ---- 2) CASO BORDE: un solo elemento ---------------------------------
    emit_insert("utec.edu.pe", "Primera insercion: los 3 bits pasan de 0 a 1.");
    emit_query("utec.edu.pe", true,
               "Caso borde 2: un solo elemento. Los 3 bits estan en 1 => SI (verdadero positivo).");

    // ---- 3) Inserciones normales (incluye colision de bits) --------------
    const std::vector<std::string> inserted = {"gradescope.com", "manim.community",
                                               "3blue1brown.com", "github.com"};
    std::unordered_set<std::string> S = {"utec.edu.pe"};
    for (const auto& k : inserted) {
        emit_insert(k, "Insercion: si un bit ya estaba en 1 se reutiliza (colision de bits).");
        S.insert(k);
    }

    // ---- 4) Consulta negativa ("definitivamente no") ---------------------
    emit_query("wikipedia.org", false,
               "Consulta negativa: al menos un bit en 0 => el elemento NO esta. Sin falsos negativos.");

    // ---- 5) Falso positivo REAL encontrado por busqueda -------------------
    // Saturamos un poco mas el filtro y buscamos una clave no insertada que de positivo.
    for (int i = 0; i < 8; ++i) {
        std::string k = "relleno-" + std::to_string(i);
        bf.insert(k);
        S.insert(k);
    }
    {
        std::ostringstream e;
        e << "\"type\":\"bulk\",\"count\":8,\"bits\":" << jarr(bf.snapshot())
          << ",\"n\":" << bf.inserted() << ",\"bits_set\":" << bf.bits_set()
          << ",\"fpr\":" << bf.false_positive_rate()
          << ",\"note\":" << jstr("Se insertan 8 claves mas: sube la carga y con ella la tasa de falsos positivos.");
        tr.event(e.str());
    }

    std::string fp_key;
    for (int i = 0; i < 200000 && fp_key.empty(); ++i) {
        std::string cand = "clave-" + std::to_string(i);
        if (S.count(cand)) continue;
        if (bf.contains(cand)) fp_key = cand;
    }
    if (!fp_key.empty()) {
        emit_query(fp_key, false,
                   "FALSO POSITIVO real: esta clave nunca se inserto, pero sus 3 bits fueron "
                   "puestos en 1 por otras claves.");
    }

    // ---- 6) CASO BORDE / PEOR CASO: filtro saturado -----------------------
    BloomFilter full(M, K);
    std::mt19937_64 rng(42);
    std::size_t added = 0;
    while (full.bits_set() < M && added < 100000) {
        full.insert("sat-" + std::to_string(rng()));
        ++added;
    }
    {
        std::ostringstream e;
        e << "\"type\":\"saturated\",\"bits\":" << jarr(full.snapshot())
          << ",\"n\":" << full.inserted() << ",\"bits_set\":" << full.bits_set()
          << ",\"fpr\":" << full.false_positive_rate()
          << ",\"note\":" << jstr("Caso borde 3 (peor caso): todos los bits en 1. El filtro responde SI a todo: inutil pero nunca falla en negativo.");
        tr.event(e.str());
    }

    // ---- 7) Validacion empirica de la tasa de falsos positivos -----------
    BloomFilter opt = BloomFilter::from_capacity(1000, 0.01);
    for (int i = 0; i < 1000; ++i) opt.insert("elem-" + std::to_string(i));
    std::size_t fp = 0, trials = 100000;
    for (std::size_t i = 0; i < trials; ++i)
        if (opt.contains("otro-" + std::to_string(i))) ++fp;
    const double fpr_emp = static_cast<double>(fp) / trials;

    std::ostringstream meta;
    meta << "\"m\":" << M << ",\"k\":" << K
         << ",\"final_n\":" << bf.inserted()
         << ",\"final_bits_set\":" << bf.bits_set()
         << ",\"final_fpr\":" << bf.false_positive_rate()
         << ",\"validation\":{\"n\":1000,\"p_target\":0.010000,\"m\":" << opt.m()
         << ",\"k\":" << opt.k()
         << ",\"fpr_teorico\":" << opt.false_positive_rate()
         << ",\"fpr_empirico\":" << fpr_emp << "}";
    tr.close(meta.str());

    // ---- Resumen por consola --------------------------------------------
    std::cout << "== Bloom Filter (implementacion propia, C++) ==\n"
              << "Demo:  m=" << M << "  k=" << K << "  n=" << bf.inserted()
              << "  bits en 1=" << bf.bits_set()
              << "  carga=" << bf.load_factor()
              << "  FPR teorica=" << bf.false_positive_rate() << "\n"
              << "Falso positivo hallado: " << (fp_key.empty() ? "(ninguno)" : fp_key) << "\n"
              << "Saturacion: " << added << " inserciones para poner los " << M << " bits en 1\n"
              << "Validacion (n=1000, p=0.01): m=" << opt.m() << " k=" << opt.k()
              << "  FPR teorica=" << opt.false_positive_rate()
              << "  FPR empirica=" << fpr_emp << "\n"
              << "Traza escrita en output/trace.json\n";
    return 0;
}
