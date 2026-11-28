# Mega Man Battle Network 4: Violet Eclipse

> **Traducción al español (borrador, 2026-11-27).** La referencia es el borrador en inglés, `docs/player-readme-draft.md`: si algo no coincide, manda el inglés. Los nombres oficiales en inglés de chips, Navis, Souls y lugares se mantienen sin traducir. El texto final, los créditos y la licencia siguen abiertos: véase `docs/release-checklist.md`.

Violet Eclipse es un parche de fans para **Mega Man Battle Network 4**. Te permite completar el juego, con todos los Double Souls, todos los chips y todos los jefes, en **una sola partida guardada**: sin segundo cartucho, sin cable link, sin tarjetas e-Reader. Hay un parche para **Red Sun** y otro para **Blue Moon**. Cada uno es un juego completo.

## Qué necesitas

- Una ROM estadounidense (USA) del juego, obtenida legalmente de tu propio cartucho. El parche la comprueba:

  | Parche | ROM | SHA-1 |
  |---|---|---|
  | `bn4_rs_overhaul.bps` | Mega Man Battle Network 4: Red Sun (USA) | `a97e96a7da03abd70f7953328e511b9fa29179f1` |
  | `bn4_bm_overhaul.bps` | Mega Man Battle Network 4: Blue Moon (USA) | `5e017c803ddc768efb8010314b46bc17e9757f71` |

- Un aplicador de parches BPS, como Flips: abre el `.bps`, elige tu ROM y guarda la copia parcheada. Conserva tu ROM original.

## Qué cambia

**Los 13 Double Souls en una sola partida**
- Los 12 Souls se pueden conseguir en cualquiera de las dos versiones. Nueve **combates amistosos** en los torneos (3 por torneo, en las rondas previas a la final) te dan el Soul del Navi al que te enfrentas: Guts, Fire, Number, Wind, Metal, Wood, Search, Thunder y Junk. Un Navi deja de aparecer cuando ya tienes su Soul. Si pierdes, puedes reintentarlo. Los otros tres (Aqua, Roll, Proto) son los Navis fijos de cada torneo, los mismos en las dos versiones.
- Un nuevo 13.º Soul, el **Duo Soul**, se obtiene al vencer a Duo (en el combate de la historia o en cualquier revancha), junto con el chip Duo (D, una sola vez). Tiene sus propios colores, un disparo cargado de puño de 180 de daño, AirShoes + FloatShoes y un escudo Reflect.
- La pantalla de estado muestra todos los Souls (R / L cambian de página), y la estrella de Souls completados exige los 12.

**Todos los chips**
- Los botines que difieren entre versiones ahora salen de la tabla de cualquiera de las dos versiones, al 50 %.
- Bass da los chips de Bass de ambas versiones (la pantalla de resultados nombra solo uno; el otro se añade en silencio).
- El Free Tournament presenta a los 12 Navis de Soul sin intercambios por link, de modo que se pueden ganar los 36 chips secretos.
- **Duo** y **Grand Prix Power**, antes chips exclusivos de eventos, ya están en el juego con nombre y descripción. Las versiones públicas no tienen imagen para ellos; véase «Problemas conocidos».
- **Z-Saver** y **Grand Prix Power** esperan cada uno en un Blue Mystery Data en un rincón apartado.

**Todos los jefes, tantas veces como quieras**
- **Revanchas en la tienda de Higsby:** los operadores de los 12 Navis de Soul esperan dentro y fuera de la tienda de Higsby. Cada uno aparece cuando posees el Soul de ese Navi. JunkMan no tiene operador: conéctate (jack in) al cubo de basura de la calle de ACDC.
  - Cada Navi sube V1 → V2 → V3 → SP → **SP+** a medida que lo vences con rango S.
- **Revanchas de Duo:** tras el final, busca la señal de Duo en Meteor Comp 1. Sube Duo → α → β → Ω → SP+ con rangos S.
- **SP+**, la nueva dificultad máxima, tras un rango S contra el SP (o Duo Ω): 4.095 HP, el daño que recibes ×1,5, velocidad ×1,25. También cubre a los jefes SP errantes. Ganar da 15.000 Zenny (80 %) o 50 BugFrags (20 %).

**Mejoras de calidad de vida**
- Un **Frequency Tuner** en la tienda de Higsby fija el nivel de virus (Auto / V1 / V2 / V3). Los Green Mystery Data siguen el mismo ajuste, así que todas las recompensas son alcanzables en cualquier partida.
- Zenny de combate ×1,5.
- HP Memory, los programas de NaviCust de Zenny, Unlocker y LocEnemy cuestan alrededor del 40 % de su precio anterior (redondeado por artículo).
- Se incluye la corrección del escenario de WoodMan (de Prof. 9).
- La Waiting Room rechaza una transferencia por link con una ranura de Navi fuera de rango, en lugar de escribirla en tu partida.

## Dónde encontrar el contenido nuevo

| Qué | Dónde |
|---|---|
| Operadores de revancha (9) y el Frequency Tuner | Dentro de la tienda de Higsby, ACDC Town |
| 2 operadores más y el cubo de basura (JunkMan) | La calle de ACDC Town; el cubo de basura está en la acera, en la esquina delantera derecha de la tienda de Higsby, bajo el toldo |
| Combates amistosos (9: tres por torneo) | DenDome Hall (Den Battle), la sala de espera del Castillo (Eagle/Hawk), el vestíbulo del Colosseum (Red Sun/Blue Moon). Están allí durante las dos primeras rondas de cada torneo (Dex solo después del combate contra AquaMan, Raika solo antes del combate contra Chaud) y desaparecen cuando posees el Soul de ese Navi |
| La señal de Duo, un marcador en forma de escudo (revanchas) | Meteor Comp 1, junto al punto de conexión (jack-in), tras el final (conéctate en el Space Center) |
| Z-Saver | Un Blue Mystery Data en un ramal sin salida rojo y oculto de Undernet 1, bajo el puente amarillo del oeste |
| Grand Prix Power | Un Blue Mystery Data en la plataforma redonda, tranquila y sin salida, de la península inferior izquierda de ACDC Area 2 |

## Partidas guardadas

Las partidas guardadas existentes de la misma versión funcionan. Los precios de la tienda se actualizan al cargar una. Haz antes una copia de seguridad de tu partida.

## Problemas conocidos

- **Versiones públicas:** los chips Duo y Grand Prix Power tienen nombre y descripción, pero no imagen.
- **Biblioteca de chips (Chip Library):** Duo y Grand Prix Power se pueden conseguir y usar en combate, pero no aparecen en la biblioteca (no hacen falta para ninguna estrella de completado).
- **Botín de Bass:** vencer a Bass da los chips de Bass de ambas versiones, aunque la pantalla de resultados solo nombra uno (el otro se añade en silencio).

Lista completa (incluidos los puertos): `docs/known-issues.md`.

## Créditos

- **Prof. 9**: la corrección del escenario de WoodMan («WoodMan Glitch Fix v2.0», de uso libre), y **TextPet** (MIT), la herramienta de texto con la que se construye el parche.
- **Kingcom**: **armips** (MIT), el ensamblador.
- **Alcaro**: **Flips** (GPL-3.0 o posterior), que crea y aplica parches `.bps`.
- **StraDaMa**: las tablas de chips de la comunidad de Mega Man Battle Network, usadas como comprobación en la compilación.
- **Sascha Becker** (*Cyberworld Endless*, MIT), el proyecto **nettai** y la comunidad de desensamblado **dism-exe**: los nombres de símbolos y las notas del motor que hicieron posible encontrar las direcciones.
- Quienes subieron a GameFAQs las partidas guardadas de GameShark de la comunidad, usadas para las pruebas.
- **Jeffrey Pfau** y los colaboradores de mGBA, cuyo emulador usan las compilaciones de prueba.

*Mega Man Battle Network* es una marca registrada de **Capcom**. Violet Eclipse es un parche de fans no oficial, sin relación con Capcom ni respaldado por ella. No contiene datos de Capcom: tú aportas tu propia ROM. La licencia del propio parche aún está por elegir.

Lista completa con licencias: `docs/credits.md`.
