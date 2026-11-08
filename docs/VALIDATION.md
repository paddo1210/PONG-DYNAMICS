# Tatsächliche Prüfungen 0.2 — 2026-11-05

Unveränderte Ergebnisse: `test-results.txt`, `feature-test-results.txt`, `host-test-results.txt`. Buildprotokoll: `build-log.txt`. Die Testprogramme melden Fehler per Exitcode; sie sind als Quellcode und Windows-Binaries enthalten.

## DSP-Basistests

- Neutraler Impuls/Sinus/Pseudozufallsrauschen-Mix, bewegte/überlappende/getrennte Fenster, Stereo, 44,1/48/88,2/96 kHz. Ausgleich der gesamten 1184-Sample-Latenz, Toleranz 2e-6 FS. Eingangspegel unterhalb der Limitergrenze.
- Volle Fensterüberlappung mit gemeinsamem +6-dB-Gain gegen analytische Referenz, Toleranz 3e-6 FS.
- Extreme Dynamik-/Drive-/Colour-Einstellungen; endliche Ausgabe; instrumentiertes `operator new`: keine Allokationen in den getesteten DSP-Schleifen.
- Blockeinteilung 32/64/128/256/512/1024 bei identisch samplegetakteten Zielwerten; identische Wellenformen.
- VELVET/IRON und positive/negative Transientenbearbeitung erzeugen unterschiedliche Wellenformen.
- Attack-Aufbau, Release nach drei Sekunden Stille, NaN/Inf-Abfangung und nachfolgende Stille.
- 60 Sekunden synthetisches Stereosignal / 48 kHz; Zeitmessung ist Offline-Walltime, kein Live-CPU-Meter.

## Neue Funktionsprüfungen

- Butterworth-Cuts bei vier Sampleraten, vier Ordnungen und beiden Richtungen. Tatsächlich durch die Filter geschickt: Sinus am 1-kHz-Cutoff und an der bilinear transformierten halben/doppelten Frequenz. Abgleich mit −3,0103 dB bzw. Butterworth-Sollabsenkung. Akzeptanz <0,15 dB; gemessen maximal rund 0,0071 dB. Das Prüfsignal wird doppelt präzise erzeugt, damit sein numerischer Rauschboden die etwa 96-dB-Sperrabsenkung nicht verdeckt.
- Oversampling: 9-kHz-Sinus, Messung der auf 3 kHz gefalteten fünften Harmonischen gegen dieselbe nicht oversampelte Sättigung. Dieser einzelne Test zeigt deutliche Reduktion; er ist keine pauschale Alias-Unterdrückungsangabe über alle Frequenzen/Pegel.
- Limiter: große asymmetrische Überlastspitzen bleiben unter der gewählten Samplegrenze. Kein Intersample-/True-Peak-Test.
- MIDI: Last-note-Priorität, Rückfall auf gehaltene Note, Note-On Velocity 0, CC.
- Externer Detektor erzeugt GR; ADAPT reduziert Drive, HOLD hält Zustände, Stille führt zur Erholung.
- PLAY + Matrix + Hall bei Blockgrößen 32 und 1024 identisch; Physik/Klang unabhängig von GUI-Aufrufen konstruiert.
- Alle drei Hallpositionen erzeugen endliche, nicht stumme Nachhallfahnen.
- Kollisionen erzeugen Pulse, verpasste Bälle verändern Punkte.
- Negative Matrixtiefe senkt den Drive wie vorgesehen; ein Zielwechsel überblendet die Routinggewichte.
- Gemeinsamer Extremtest bei allen vier Sampleraten: Drive/Output/GLUE/Master jeweils bis +48 dB, volle Colour-, PLAY-, ADAPT- und Hallbearbeitung. Endliche Ausgaben und Einhaltung der Limitergrenze.
- Zusätzlicher Benchmark mit fünf aktiven Colour-Stufen, GLUE und PLAY für zehn Sekunden Stereo / 48 kHz. Nicht mit einer garantierten maximalen Callbackzeit verwechseln.

## JUCE-Prozessor-Harness

- 165 Hostparameter; MIDI deklariert; ±48-dB-Endpoints; Parameterserialisierung.
- Mono/Stereo, alle vier Sampleraten und sechs Puffergrößen, Impuls bis über die gemeldete Latenz hinaus geprüft.
- Externer Mono-Sidechain-Bus neben Stereo-Haupteingang, tatsächlich getrennte Audiopuffer verarbeitet.
- MIDI-Ereignisse an Samplepositionen innerhalb des Hostblocks verarbeitet.
- Ballpositionen aus Host-State wiederhergestellt; weitere Simulation identisch fortgesetzt.
- Vier Bedienseiten mit dem realen JUCE-Editor offscreen gerendert und visuell geprüft; PNGs im Paket.

## Vergleichsdateien

`Audio/01-Neutral.wav` bis `04-Pong-Play.wav`: identische synthetische Quelle mit Bass, perkussiven Impulsen, Rauschen und Sinusflächen; sechs Sekunden Stereo, 48 kHz/16 Bit. Vier Modi neutral/statisch/adaptiv/PLAY; auf denselben RMS-Pegel gebracht und vor PCM-Ausgabe auf Spitzenreserve geprüft. Genaue Pegel in `audio-render-results.txt`.

Es handelt sich um RMS-, nicht LUFS-Angleichung. Keine natürlichen Sprach-/Gesangsaufnahmen oder vollständigen fremden Musikmischungen wurden verwendet. Die Klangdateien wurden erzeugt und numerisch geprüft; ein menschlicher Hörvergleich wird nicht behauptet.

## Nicht durchgeführt

Ableton Live 11 Integration, realer Mikrofon-/MIDI-Controller-Test, native Hardware-Audiolatenz, pluginval/Steinberg-Validator, mehrstündiger Stresstest, Debian-/macOS-Build, systematischer Test aller möglichen Automation-/Presetwechsel und ein breiter subjektiver Musik-Hörvergleich. GUI-Seiten wurden gerendert; damit ist nicht jede Mausgeste im nativen Host verifiziert.

Abschließende Messwerte stehen in den Protokollen. Benchmarkwerte schwanken mit konkurrierender Last. Klangunterschiede, numerische Stabilität und ein erfolgreicher Build sind kein Beweis für Klangqualität oder perfekte Host-Kompatibilität.
