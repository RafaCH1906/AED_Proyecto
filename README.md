# Bloom Filter — Anima tu Estructura de Datos

**CS2023 — Algoritmos y Estructuras de Datos · UTEC · 2026-2**
Proyecto Final 1 · Prof. Víctor Racsó Galván Oyola

**Integrantes:** Paul Maguiña · Rafael Choque · Enrique Torres

Video educativo, al estilo 3Blue1Brown, sobre el **Bloom Filter**. La estructura está
implementada desde cero en **C++17** y la animación se hizo con **Manim Community**.

---

## La animación es real, no simulada

```
bloom_filter.cpp + main.cpp  ──►  output/trace.json  ──►  animacion.py  ──►  video .mp4
     (estructura real)              (traza de pasos)        (solo dibuja)
```

1. `main.cpp` ejecuta el guion del video **contra nuestra implementación** del Bloom Filter
   y registra cada paso en `output/trace.json`: la clave, los `k` índices que devolvió cada
   función hash, el valor de cada bit **antes** de la operación, el vector de bits completo,
   el resultado de cada consulta y la tasa de falsos positivos.
2. `animacion.py` **no contiene lógica de la estructura**. Solo lee la traza y dibuja lo que
   pasó. Si se cambian `m`, `k` o las claves en el C++, cambia el video sin tocar Python.

El falso positivo que aparece en el video (`clave-16`) no está escrito a mano. Lo encontró
el programa buscando una clave **no insertada** a la que el filtro le responde que sí.

## Archivos

```
AED_Proyecto/
├── bloom_filter.hpp   # interfaz del TDA
├── bloom_filter.cpp   # implementación: vector de bits propio + hashing propio
├── main.cpp           # guion del video: ejecuta la estructura y escribe la traza
├── animacion.py       # escena de Manim (BloomFilterScene), lee output/trace.json
└── README.md
```

## Software requerido

| Software | Versión usada | Para qué |
|---|---|---|
| g++ (MinGW-w64 / WinLibs, o GCC en Linux/macOS) | C++17 (probado con GCC 16.1) | Compilar la estructura |
| Python | 3.11 o superior (probado con 3.12) | Ejecutar Manim |
| Manim Community | 0.21.0 | Renderizar la animación |

`animacion.py` **no usa `MathTex`**, así que **no necesita LaTeX**. Manim 0.21 codifica el
video con PyAV, así que tampoco hace falta instalar FFmpeg aparte.

En Windows, g++ se instala con:

```powershell
winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT
```

## Cómo compilar y generar el video

Todos los comandos se ejecutan **desde la raíz del repositorio**, porque la animación abre
`output/trace.json` con una ruta relativa.

### 1. Compilar y ejecutar la estructura (genera la traza)

```bash
g++ -std=c++17 -O2 main.cpp bloom_filter.cpp -o bloom_demo
mkdir output
./bloom_demo          # en Windows: .\bloom_demo.exe
```

Salida esperada:

```
== Bloom Filter (implementacion propia, C++) ==
Demo:  m=64  k=3  n=13  bits en 1=27  carga=0.421875  FPR teorica=0.095012
Falso positivo hallado: clave-16
Saturacion: 117 inserciones para poner los 64 bits en 1
Validacion (n=1000, p=0.01): m=9586 k=7  FPR teorica=0.0100345  FPR empirica=0.01017
Traza escrita en output/trace.json
```

### 2. Instalar Manim

```bash
python -m venv venv
# Windows (PowerShell):
#   Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
#   .\venv\Scripts\Activate.ps1
# Linux / macOS:
#   source venv/bin/activate
pip install -U pip setuptools wheel
pip install manim
```

### 3. Renderizar

```bash
manim -ql animacion.py BloomFilterScene   # borrador rápido (480p15)
manim -qh animacion.py BloomFilterScene   # versión final (1080p60)
```

El video queda en `media/videos/animacion/1080p60/BloomFilterScene.mp4` y dura unos 4:03.

## Formato de `output/trace.json`

```jsonc
{
  "events": [
    { "type": "init",   "m": 64, "k": 3, "bits": [0, ...] },
    { "type": "insert", "key": "utec.edu.pe",
      "probes": [{"hash_id": 0, "raw": 1490167..., "index": 23, "bit_before": false}, ...],
      "bits": [...], "n": 1, "bits_set": 3, "fpr": 0.0001 },
    { "type": "query",  "key": "clave-16", "probes": [...], "bits": [...],
      "result": true, "really_in": false, "verdict": "FALSO_POSITIVO" },
    { "type": "bulk",      "count": 8, "bits": [...], "n": 13, "bits_set": 27, "fpr": 0.095 },
    { "type": "saturated", "bits": [1, ...], "n": 117, "bits_set": 64, "fpr": 1.0 }
  ],
  "meta": { "m": 64, "k": 3, "final_n": 13, "final_bits_set": 27, "final_fpr": 0.095012,
            "validation": { "n": 1000, "p_target": 0.01, "m": 9586, "k": 7,
                            "fpr_teorico": 0.010034, "fpr_empirico": 0.01017 } }
}
```

- `bit_before` permite distinguir un bit nuevo de uno que ya estaba en 1 (colisión).
- `verdict` toma uno de tres valores: `verdadero_positivo`, `FALSO_POSITIVO` o `definitivamente_no`.

## La estructura

**TDA representado:** Conjunto (*Set*) **probabilístico**, con dos operaciones:

- `insert(x)`: agrega `x`.
- `contains(x)`: si devuelve `false`, `x` **definitivamente no está** (no hay falsos
  negativos). Si devuelve `true`, `x` **probablemente está** (puede ser un falso positivo).

No guarda las claves, solo `m` bits y `k` funciones hash. Por eso **no admite `delete`**:
apagar un bit podría borrar también otras claves.

### Detalles de la implementación

- **Vector de bits propio:** `std::vector<uint64_t>`. El bit `i` está en la palabra `i >> 6`,
  en la posición `i & 63`.
- **Hashing propio:** FNV-1a de 64 bits con semilla, más una mezcla de avalancha tipo
  `splitmix64`. Las `k` posiciones se sacan con **Kirsch–Mitzenmacher**:
  `h_i(x) = h1(x) + i·h2(x) + i²  (mod m)`, con `h2` forzado a impar.
- **Dimensionamiento óptimo** (`BloomFilter::from_capacity(n, p)`):
  `m = ⌈-n·ln p / (ln 2)²⌉`, `k = round((m/n)·ln 2)`.
- **Tasa de falsos positivos:** `p ≈ (1 - e^(-k·n/m))^k`.
- **Parámetros del video:** `m = 64` bits y `k = 3`, que en pantalla forman una cuadrícula de 4 × 16.

### Casos borde que muestra el video

1. **Filtro vacío:** cualquier consulta responde "definitivamente no".
2. **Un solo elemento:** consultar la clave insertada da un verdadero positivo.
3. **Filtro cargado:** con 27 de 64 bits en 1 aparece un falso positivo real (`clave-16`).
   Llevado al extremo (la traza lo registra en el evento `saturated`), 117 inserciones
   encienden los 64 bits y el filtro responde "sí" a todo.

### Complejidad

`k` = número de funciones hash, `L` = largo de la clave, `m` = número de bits.

| Operación | Tiempo | Espacio extra | Nota |
|---|---|---|---|
| `insert(x)` | Θ(k + L), es decir **O(1)** respecto a `n` | O(1) | siempre hace los `k` sondeos |
| `contains(x)` | O(k + L), es decir **O(1)** respecto a `n` | O(1) | se detiene en el primer bit en 0 |
| `delete(x)` | — | — | no se admite por diseño |
| Construcción | Θ(m / 64) | **Θ(m) bits** | no almacena las claves |

Ningún costo depende de `n`. Con dimensionamiento óptimo, una tasa de falsos positivos de
1 % cuesta unos **9.6 bits por elemento**, sin importar el largo de las claves.

**Validación empírica** (`main.cpp`): con `n = 1000` y un objetivo de `p = 0.01` se obtiene
`m = 9586` y `k = 7`. En 100 000 consultas de claves no insertadas, la FPR medida fue
**1.02 %**, frente a **1.00 %** teórica.

## Contribuciones

| Integrante | Responsabilidad |
|---|---|
| **Enrique Torres** | Implementación en C++ (vector de bits, hashing, dimensionamiento) y generación de la traza (`bloom_filter.hpp`, `bloom_filter.cpp`, `main.cpp`) |
| **Paul Maguiña** | Escena de Manim (`animacion.py`), render en 1080p60 y montaje del audio |
| **Rafael Choque** | README, informe en PDF y organización del repositorio |

Los tres narramos el video: Rafael la introducción, Enrique la demostración y Paul los
casos finales y la complejidad.

## Referencias

1. B. H. Bloom, "Space/time trade-offs in hash coding with allowable errors", *Communications of the ACM*, 13(7), 1970.
2. A. Kirsch y M. Mitzenmacher, "Less hashing, same performance: building a better Bloom filter", *ESA*, 2006.
3. Manim Community — https://docs.manim.community/en/stable/
4. 3Blue1Brown (referencia de estilo) — https://www.3blue1brown.com/
