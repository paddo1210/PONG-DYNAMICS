# PONG DYNAMICS 0.2.0

Adaptiver spektraler Charakterprozessor mit spielbarer Pong-Ansicht. Windows x64 VST3 und Standalone, C++20/JUCE 8.0.12/CMake. Eigenständige Algorithmen, keine originalgetreue Fairchild-/Distressor-/EMT-/HEAT-Emulation.

## Installation / Update in Ableton Live 11

1. Live schließen. Den **vollständigen Ordner** `Windows/VST3/PONG DYNAMICS.vst3` nach `C:\Program Files\Common Files\VST3\` kopieren und die alte Version ersetzen. Nicht nur die innere Binärdatei kopieren.
2. Live öffnen, VST3-Systemordner aktivieren und neu scannen.
3. Plugin auf eine Audiospur ziehen. **PONG DEMO** aktiviert eine hörbare Kombination aus Dynamik, adaptiver Matrix, Hall und PLAY.
4. Alternativ `Windows/Standalone/PONG DYNAMICS.exe` starten. Audio-Gerät und Puffer in den JUCE-Audioeinstellungen wählen.

Es wurde kein systemweiter Pluginordner verändert. Bei fehlenden MSVC-Laufzeitbibliotheken wird das Microsoft Visual C++ 2015–2022 Redistributable x64 benötigt. Der Build fordert weder AVX noch AVX2.

**Kein Live-11-Hosttest durchgeführt.** VST3-Build, direktes JUCE-Prozessor-Testprogramm und Offline-DSP-Tests sind durchgeführt; reale DAW-Routing-/Audiogeräte-Tests bleiben separat nötig. Externe Sidechain ist im VST3 vorgesehen. Die Standard-Standalone-Gerätekonfiguration bietet keine zusätzliche Sidechain-Patchbay.

## Neu gegenüber 0.1

- PLAY: drei bewegte Frequenzkörper, zwei ziehbare Schläger, Wand-/Ball-/Schlägerkollisionen, Punkte bei verpassten Bällen. Bewegung und Klang werden vom Audiotakt gesteuert, nicht vom GUI-Timer.
- Acht Modulationsrouten mit Quellwahl, Zielmodul, Zielparameter, positiver/negativer Tiefe und individueller Glättung.
- ADAPT, INTERACTION, RECOVERY, HOLD; sichtbare Drive-Rücknahme pro Modul.
- Externer Sidechain-Bus; MIDI-Noten, Velocity, Gate und frei gewählter CC als Steuerquellen.
- Plattenartiger FDN-Hall mit Pre-Delay, Decay, Damping, Send/Return und PRE/POST/SEND-Routing.
- GLUE: oversampelte Sättigung, leichte Summenkompression und Trim.
- Feste vierfache FIR-Überabtastung in allen fünf Sättigungsstufen und GLUE.
- Unabhängige Butterworth-Low-/High-Cuts, 12/24/48/96 dB pro Oktave. Kein angeblicher Brickwall-Modus.
- Stereo-gekoppelter Lookahead-Sample-Peak-Limiter mit einstellbarer Ausgangsgrenze.
- Input/Drive, Modul-Output, Transient/Sustain, GLUE-Trim und Master-Output bis **±48 dB**; maximaler Dynamikeingriff bis 48 dB.

## Vier Bedienseiten

### PONG / PLAY

Die Spielansicht bleibt auch auf den anderen Seiten sichtbar. PLAY schaltet die Audiotakt-Simulation ein. EDIT erlaubt das manuelle Verschieben der inneren Frequenzfenster. Die Pegelkarten wählen ein Modul und öffnen seine präzisen Regler.

**X:** logarithmische Grundfrequenz des bewegten Fensters. Die getönte Fläche und Frequenzzahlen zeigen das tatsächlich wirksame Fenster nach Matrixmodulation. **Y:** im PLAY-Modus ein Drive-Offset von ±48 dB, multipliziert mit `Collision / Drive depth`; im EDIT-Modus der manuelle Drive-Wert. Die Matrix und ADAPT können den wirksamen Drive zusätzlich verändern. Schläger-Y folgt dem manuellen Drive von LOW/HIGH.

`Collision / Drive depth` skaliert Drive-Auslenkung und Kollisionsimpulse. Die Frequenzbewegung wird unabhängig davon durch PLAY und die beiden Wandgrenzen bestimmt. Tiefe 0 stoppt also nicht die Frequenzbewegung; dafür EDIT wählen oder HOLD verwenden.

Schläger ziehen: X verändert die jeweilige Bewegungsgrenze und LOW-End-/HIGH-Startfrequenz; Y verändert den Drive. Die effektiven Wandpositionen können enger als die eingestellten Grenzwerte sein, damit die volle Breite jedes inneren Fensters zwischen 20 Hz und 20 kHz bleibt. Die sichtbaren Schläger stehen an den effektiven Wänden. Bei gleich gewählten Grenzen bleibt praktisch keine horizontale Bewegung übrig.

Tempo-Sync verwendet Host-BPM, ersatzweise 120 BPM. Die Simulation läuft auch bei gestopptem Transport weiter, sofern der Host Audioblöcke liefert. `Speed` skaliert die Geschwindigkeit. Seed-Änderung initialisiert reproduzierbare Bewegungsrichtungen. Der gespeicherte Plugin-State enthält den aktuellen Bewegungszustand, nicht nur den Seed.

Kollisionen erzeugen begrenzte, abklingende Transientenverstärkung, mehr IRON-Gewichtung und zusätzlichen Plate-Send. Werden die Schläger verfehlt, zählt die Gegenseite einen Punkt und der Ball startet in der Mitte neu. Andere Frequenzkörper stoßen elastisch voneinander ab. Die Regeln sind Pong-inspiriert, kein originalgetreuer Nachbau eines historischen Spiels.

### MODULES

Fünf überlappende Frequenzmodule, getrennte Start-/Endfrequenzen für die inneren Fenster. Width Lock bewahrt beim grafischen Verschieben deren Oktavbreite; die Zahlenregler lassen sich weiterhin unabhängig einstellen. End kleiner als Start wird DSP-seitig auf Start begrenzt.

Input/Drive wirkt vor Detektor und Dynamik, Output danach. Threshold/Ratio/Knee bestimmen die Kompressionskennlinie. Max change begrenzt Kompression und Transient/Sustain-Gain, nicht den Summenausgang. Fast-/Slow-Hüllkurven steuern Transient/Sustain; dies ist keine bloße Threshold-Modulation.

VELVET verwendet eine weichere effektive Kennlinie und programmabhängig verlängerte Erholung. IRON greift fester ein; CHARACTER blendet diese Eigenschaften sowie zwei eigene Sättigungskurven. Colour=0 deaktiviert die Nichtlinearität. Die FIR-Parallelverzögerung bleibt erhalten. Es gibt kein automatisches Makeup-Gain.

Detektorwahl: eigenes Band, Breitband, externer Sidechain, anderes Band, Plate-Return oder MIDI-Velocity × Gate. Eine fremde Detektorquelle verändert nur die Dynamiksteuerung, nicht den Audioeingang des Moduls.

LOW/HIGH: `Calibrated cut mix` blendet einen zusätzlichen unabhängigen Highpass/Lowpass ein. **Für den kalibrierten vollständigen Cut auf 1 stellen**, Modul-Mix auf 1 und Bypass aus. Der Cut liegt nach der parallelen Bandsumme. Cut-Mix 0 lässt das normale Randmodul stehen; dessen Output wirkt als breites Shelf über seinen normalisierten spektralen Anteil. Dieser Shelf ist kein unabhängig kalibrierter klassischer EQ-Shelf.

`Cut slope` zeigt 12/24/48/96 dB/Okt. an. Intern wird der historische kontinuierliche Parameter auf diese vier Stufen abgebildet und zwischen parallelen Filterketten geglättet. Die Grenzfrequenz ist der gemeinsame −3,0103-dB-Punkt. Obergrenze der Cut-Frequenz: 0,45 × Samplerate. 96 dB/Okt. ist ein steiler IIR-Cut, kein idealer Brickwall-Filter.

### ADAPTIVE MATRIX

Acht Zeilen: Quelle → Modul → Ziel, dazu signed Depth und Smoothing. Negative Tiefe kehrt die Polarität um. Quellen: fünf Bandenergien, fünf Transientenverläufe, Breitband, Sidechain, MIDI-Note/Velocity/Gate/CC, Hallenergie und Kollisionen. Ziele: Drive, Threshold, Frequenz, Character, Plate-Send.

Pro Route: Drive/Threshold bis ±24 dB, Frequenz bis ±2 Oktaven; mehrere Routen werden summiert und anschließend auf erlaubte Parametergrenzen begrenzt. Bandenergiequellen werden zwischen −60 und 0 dBFS auf 0…1 abgebildet. Transienten und MIDI-Note können bipolar sein. MIDI-Noten liegen relativ zu Note 60 im Bereich ±24 Halbtöne, begrenzt auf −1…1.

MIDI-Priorität: zuletzt angeschlagene, noch gehaltene Note. Beim Loslassen Rückkehr zur zuvor gehaltenen Note. Note-On mit Velocity 0 ist Note-Off; CC120/123 löscht gehaltene Noten. Alle MIDI-Kanäle werden gemeinsam ausgewertet. Sustainpedal, MPE und Pitchbend sind nicht implementiert. Note/Velocity/Gate müssen in der Matrix oder als Detektor ausgewählt werden; sie spielen keinen eigenen Synthesizer.

ADAPT erkennt anhaltend hohen Bandpegel oder hohe GR und nimmt bis zu 24 dB Drive sowie bis zu 75 % der Modulationstiefe zurück. Hysterese, Eingriffszeit und begrenzte Rückkehrgeschwindigkeit verhindern schnelles Hin-und-her-Schalten. RECOVERY bestimmt die Erholung. HOLD friert Bewegungen, Kollisionszustand, adaptive Zustände und geglättete Matrixwerte ein. Manuelle Parameter bleiben veränderbar. Dies ist ein deterministischer Regler, keine KI und keine Bewertung musikalischer Qualität.

### PLATE / GLUE / LIMIT

PRE: Hall wird vor den Frequenzmodulen zugemischt. POST: Hall als Insert nach der Dynamik, Return ist Wet-Anteil. SEND: paralleler Hall nach der Dynamik, Return ist zusätzlicher Rücklauf. Send regelt den Hall-Eingangspegel. Auch Send=0 kann bei POST und Return>0 den trockenen Anteil absenken, weil Return dort ein Wet/Dry-Mix ist.

Zwei laufende Hallinstanzen ermöglichen geglättete Positionswechsel. Hallfahnen bleiben beim Positionswechsel erhalten, werden aber beim Neuinitialisieren/Sampleratenwechsel zurückgesetzt. Der Hall ist ein eigener dichter, metallisch gefärbter FDN-Algorithmus, kein EMT-140-Schaltungs-/Impulsmodell und kein EMT-250-Modell.

GLUE-Kompression: weiche, begrenzte Peak-Verdichtung bis 6 dB. GLUE-Sättigung hat eine eigene 4x-Stufe. GLUE Trim und Master Output wirken vor dem abschließenden Limiter.

**Limiter immer aktiv, auch bei Global Bypass.** 128 Samples Lookahead, gekoppelte Stereoregelung, 60-ms-Erholung, Standardgrenze −1 dBFS. Er begrenzt digitale Sample-Spitzen; **keine True-Peak-/Intersample-Garantie**. Neutraler Nulltest gilt unterhalb der Limitergrenze. Global Bypass blendet zum gleich verzögerten Original, der Limiter bleibt als Ausgangsschutz bestehen.

## Sidechain und MIDI in der DAW

Für Mikrofon-Ducking zunächst **MIC DUCK** wählen und das Mikrofon im Host auf den externen Sidechain des Plugins routen. Bei leerem externen Bus bleibt dessen Detektor still. Mikrofon-Monitoring und Feedbackschutz werden im Host eingestellt.

Für Notensteuerung **MIDI MOTION** wählen und eine MIDI-Spur zum MIDI-Eingang dieser Plugininstanz routen, soweit der Host dies anbietet. Noten verschieben BODY, Velocity erhöht dessen Drive, der gewählte CC steuert PRESENCE-Character. Konkrete Live-Menüs und die Kombination aus externem Sidechain/MIDI müssen noch in Live geprüft werden.

## Latenz, Leistung und State

Feste Latenz **1184 Samples**: 1024 STFT + 16 Band-Oversampling + 16 GLUE-Oversampling + 128 Limiter. 24,67 ms bei 48 kHz; alle trockenen Parallel-/Bypasspfade werden ausgeglichen. Dies ist ein kreativer Effekt mit spürbarer Monitoring-Latenz, keine Zero-Latency-Lösung.

Vierfaches Oversampling kostet Rechenzeit. Die dokumentierten Offline-Benchmarks sind keine DAW-CPU-Garantie; insbesondere 96 kHz, kleine Puffer und mehrere Instanzen sind auf älteren CPUs separat zu prüfen.

Gespeichert: 165 Parameter, Matrix, Seed, Ballpositionen/-geschwindigkeiten, Scores, Kollisionszustand, Steuerschrittphase und adaptive/Matrix-Zustände. Nicht gespeichert: laufende Audioringpuffer/Hallfahnen und gehaltene MIDI-Noten. Parameterübernahmen werden geglättet. Bisherige Parameter-IDs bleiben erhalten; **bestehende normalisierte Host-Automationskurven können wegen der erweiterten ±48-dB-Skala andere Pegel ergeben**. Alte Projekte vor dem Update sichern.

## Build und Prüfung

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Benötigt CMake >=3.22, VS2022 C++ Build Tools und Windows SDK. JUCE 8.0.12 liegt unter `vendor/JUCE`; dessen Lizenzbedingungen in `vendor/JUCE/LICENSE.md` beachten. VST3/Standalone-Ausgabe unter `build/PongDynamics_artefacts/Release`.

Nur DSP: `cmake -S . -B build-tests -DPONG_PLUGIN=OFF`, danach Build/CTest wie oben. Der Quellcode ist plattformübergreifend angelegt; Debian-/macOS-Builds wurden nicht durchgeführt.

`Windows/Tests` enthält ausführbare Testprogramme. `Audio` enthält vier auf identischen RMS-Pegel gebrachte synthetische Vergleiche, keine fremden Musikaufnahmen. Weitere Grenzen und tatsächliche Messungen: `docs/ARCHITECTURE.md`, `docs/VALIDATION.md` und die unveränderten Testprotokolle.
