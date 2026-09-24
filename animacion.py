from manim import *
import json

class BloomFilterScene(Scene):
    def construct(self):
        # ==========================================
        # 1. PORTADA Y NOMBRES
        # ==========================================
        titulo_principal = Text("Bloom Filter", font_size=60, color=BLUE)
        curso = Text("Algoritmos y Estructuras de Datos", font_size=24, color=GRAY).next_to(titulo_principal, UP)
        nombres = Text("Por: Paul Maguiña, Rafael Choque y [Nombre 3]", font_size=24).next_to(titulo_principal, DOWN, buff=0.5)

        self.play(Write(curso), Write(titulo_principal))
        self.play(FadeIn(nombres))
        self.wait(8) # 8 segundos para la portada
        self.play(FadeOut(curso), FadeOut(titulo_principal), FadeOut(nombres))

        # ==========================================
        # 2. DEFINICIÓN Y TDA
        # ==========================================
        def_titulo = Text("¿Qué es un Bloom Filter?", font_size=40, color=YELLOW).to_edge(UP)
        def_1 = Text("Estructura de datos probabilística de alta eficiencia.", font_size=24)
        def_2 = Text("Responde a la pregunta: \"¿El elemento X pertenece al conjunto S?\"", font_size=24).next_to(def_1, DOWN, buff=0.3, aligned_edge=LEFT)
        def_3 = Text("TDA: Representa un Conjunto (Set) con operaciones de:", font_size=24).next_to(def_2, DOWN, buff=0.3, aligned_edge=LEFT)
        def_4 = Text("   • Inserción (insert)\n   • Consulta de pertenencia (contains)", font_size=24).next_to(def_3, DOWN, buff=0.1, aligned_edge=LEFT)

        grupo_def = VGroup(def_1, def_2, def_3, def_4).center()

        self.play(Write(def_titulo))
        self.play(FadeIn(grupo_def, lag_ratio=0.5))
        self.wait(15) # 15 SEGUNDOS PARA EXPLICAR LA DEFINICIÓN
        self.play(FadeOut(def_titulo), FadeOut(grupo_def))

        # ==========================================
        # 3. REGLAS Y PROPIEDADES
        # ==========================================
        reglas_titulo = Text("Propiedades Clave", font_size=40, color=YELLOW).to_edge(UP)
        regla_1 = Text("1. Un SÍ probablemente es correcto (puede ser Falso Positivo).", font_size=24)
        regla_2 = Text("2. Un NO es definitivo (Cero Falsos Negativos).", font_size=24).next_to(regla_1, DOWN, buff=0.4, aligned_edge=LEFT)
        regla_3 = Text("3. Usa muy poca memoria (vector de m bits y k hashes).", font_size=24).next_to(regla_2, DOWN, buff=0.4, aligned_edge=LEFT)
        regla_4 = Text("4. No admite eliminación de elementos.", font_size=24).next_to(regla_3, DOWN, buff=0.4, aligned_edge=LEFT)

        grupo_reglas = VGroup(regla_1, regla_2, regla_3, regla_4).center()

        self.play(Write(reglas_titulo))
        self.play(FadeIn(grupo_reglas, lag_ratio=0.5))
        self.wait(15) # 15 SEGUNDOS PARA EXPLICAR LAS REGLAS
        self.play(FadeOut(reglas_titulo), FadeOut(grupo_reglas))

        # ==========================================
        # 4. CASOS DE USO PRÁCTICOS
        # ==========================================
        usos_titulo = Text("Usos en la Práctica", font_size=40, color=YELLOW).to_edge(UP)
        uso_sub = Text("Evita operaciones costosas cuando la respuesta es NO:", font_size=24, color=GRAY).next_to(usos_titulo, DOWN)

        uso_1 = Text("• Bases de Datos (LSM): Evita leer de disco tablas irrelevantes.", font_size=24)
        uso_2 = Text("• Navegadores: Filtra listas gigantes de URLs maliciosas.", font_size=24).next_to(uso_1, DOWN, buff=0.3, aligned_edge=LEFT)
        uso_3 = Text("• Caches Web (CDNs): Evita almacenar objetos pedidos una sola vez.", font_size=24).next_to(uso_2, DOWN, buff=0.3, aligned_edge=LEFT)

        grupo_usos = VGroup(uso_1, uso_2, uso_3).center()

        self.play(Write(usos_titulo), FadeIn(uso_sub))
        self.play(FadeIn(grupo_usos, lag_ratio=0.5))
        self.wait(15) # 15 SEGUNDOS PARA LOS EJEMPLOS
        self.play(FadeOut(usos_titulo), FadeOut(uso_sub), FadeOut(grupo_usos))

        # ==========================================
        # 5. LEER EL JSON Y CREAR LA CUADRÍCULA
        # ==========================================
        with open("output/trace.json", "r") as f:
            data = json.load(f)
        events = data["events"]

        titulo_accion = Text("Demostración Visual", font_size=40).to_edge(UP)
        sub_accion = Text("Parámetros: m = 64 bits, k = 3 funciones hash", font_size=20, color=GRAY).next_to(titulo_accion, DOWN)
        self.play(Write(titulo_accion), FadeIn(sub_accion))

        # Creamos la grilla de 64 bits
        cuadricula = VGroup()
        for i in range(64):
            cuadro = VGroup(
                Square(side_length=0.5, stroke_color=WHITE, fill_color=BLACK, fill_opacity=1),
                Text(str(i), font_size=16)
            )
            cuadricula.add(cuadro)

        cuadricula.arrange_in_grid(rows=4, cols=16, buff=0.1)
        self.play(FadeIn(cuadricula))
        self.wait(4)

        # ==========================================
        # 6. ANIMAR EVENTOS
        # ==========================================
        for evento in events:
            tipo = evento.get("type")

            if tipo not in ["insert", "query", "bulk"]:
                continue

                # --- MANEJO DEL EVENTO BULK ---
            if tipo == "bulk":
                texto_accion = Text("Insertando 8 claves de relleno simultáneas...", font_size=24, color=YELLOW).next_to(cuadricula, DOWN)
                self.play(Write(texto_accion), run_time=1.0)

                bits_snapshot = evento["bits"]
                animaciones_pintar = []
                for i, bit_val in enumerate(bits_snapshot):
                    if bit_val == 1:
                        cuadro_actual = cuadricula[i][0]
                        if cuadro_actual.get_fill_color() != BLUE:
                            animaciones_pintar.append(cuadro_actual.animate.set_fill(BLUE, opacity=1))

                if animaciones_pintar:
                    self.play(*animaciones_pintar, run_time=2.0)

                self.wait(7) # 7 segundos para explicar la saturación
                self.play(FadeOut(texto_accion), run_time=1.0)
                continue

            # --- MANEJO NORMAL DE INSERT Y QUERY ---
            key = evento["key"]

            if tipo == "insert":
                texto_accion = Text(f"Insertando: {key}", font_size=24, color=YELLOW).next_to(cuadricula, DOWN)
                self.play(Write(texto_accion), run_time=1.0)

                animaciones_pintar = []
                for probe in evento["probes"]:
                    indice = probe["index"]
                    cuadro_actual = cuadricula[indice][0]
                    animaciones_pintar.append(cuadro_actual.animate.set_fill(BLUE, opacity=1))

                if animaciones_pintar:
                    self.play(*animaciones_pintar, run_time=1.5)

                self.wait(6) # 6 Segundos para explicar cada inserción
                self.play(FadeOut(texto_accion), run_time=0.8)

            elif tipo == "query":
                texto_accion = Text(f"Consultando: {key}", font_size=24, color=YELLOW).next_to(cuadricula, DOWN)
                self.play(Write(texto_accion), run_time=1.0)

                animaciones_borde = []
                for probe in evento["probes"]:
                    indice = probe["index"]
                    cuadro_actual = cuadricula[indice][0]
                    color_borde = GREEN if probe["bit_before"] else RED
                    animaciones_borde.append(cuadro_actual.animate.set_stroke(color_borde, width=4))

                if animaciones_borde:
                    self.play(*animaciones_borde, run_time=1.5)

                veredicto_limpio = evento.get("verdict", "").replace("_", " ").upper()

                if "FALSO POSITIVO" in veredicto_limpio:
                    color_ver = RED
                elif "NO" in veredicto_limpio:
                    color_ver = GREEN
                else:
                    color_ver = BLUE

                texto_res = Text(veredicto_limpio, font_size=22, color=color_ver).next_to(texto_accion, DOWN)
                self.play(Write(texto_res), run_time=1.0)
                self.wait(8) # 8 Segundos para leer y explicar el veredicto (falso positivo, etc)

                restaurar = [cuadricula[p["index"]][0].animate.set_stroke(WHITE, width=1) for p in evento["probes"]]
                self.play(FadeOut(texto_accion), FadeOut(texto_res), *restaurar, run_time=1.0)

        self.wait(3)
        self.play(FadeOut(cuadricula), FadeOut(titulo_accion), FadeOut(sub_accion))

        # ==========================================
        # 7. ANÁLISIS DE COMPLEJIDAD
        # ==========================================
        comp_titulo = Text("Análisis de Complejidad", font_size=40, color=BLUE).to_edge(UP)

        t_titulo = Text("Complejidad Temporal: O(k + L)", font_size=32, color=YELLOW)
        t_1 = Text("• L: Largo de la clave para calcular el Hash base.", font_size=24)
        t_2 = Text("• k: Cantidad de funciones hash aplicadas en O(1).", font_size=24)
        t_3 = Text("• Gran ventaja: Es O(1) respecto al número de elementos 'n'.", font_size=24)
        grupo_t = VGroup(t_titulo, t_1, t_2, t_3).arrange(DOWN, aligned_edge=LEFT, buff=0.2)

        e_titulo = Text("Complejidad Espacial: O(m)", font_size=32, color=YELLOW)
        e_1 = Text("• m: Cantidad total de bits del arreglo.", font_size=24)
        e_2 = Text("• Ventaja: Independiente del tamaño original de las claves.", font_size=24)
        grupo_e = VGroup(e_titulo, e_1, e_2).arrange(DOWN, aligned_edge=LEFT, buff=0.2)

        grupo_complejidad = VGroup(grupo_t, grupo_e).arrange(DOWN, aligned_edge=LEFT, buff=0.8).center()

        self.play(Write(comp_titulo))
        self.play(FadeIn(grupo_t, lag_ratio=0.5))
        self.wait(15) # 15 Segundos para explicar el tiempo
        self.play(FadeIn(grupo_e, lag_ratio=0.5))
        self.wait(15) # 15 Segundos para explicar el espacio
        self.play(FadeOut(Group(*self.mobjects)))