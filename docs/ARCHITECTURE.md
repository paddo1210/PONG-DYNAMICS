# Architektur 0.2

## Audio-DAG

Input → optionaler PRE-Plate → WOLA-Analyse → fünf parallele Bandprozessoren → Differenzsummierung → unabhängige LOW-/HIGH-Cuts → POST-/SEND-Plate → GLUE → Master Trim → Lookahead-Limiter.

Die Audioverbindungen sind feste, auswählbare rückkopplungsfreie Pfade. Es gibt keine frei programmierbare seriell/parallele 5×5-Audiomatrix. Die frei editierbare achtzeilige Matrix ist ausdrücklich eine **Modulationsmatrix**. Das interne Hall-FDN besitzt natürlich seinen eigenen gedämpften algorithmischen Feedbackpfad; dieser ist nicht Teil der Benutzerroutingmatrix.

## Frequenzfenster und Rekonstruktion

STFT N=1024, Hop=256, Sinusfenster für Analyse und Synthese; Summe überlappender Fensterquadrate =2, Synthese-Faktor 0,5. Eigene feste Radix-2-FFT. Logarithmisch steuerbare Masken `w(f)=1/(1+(lo/f)^4)/(1+(f/hi)^4)`, bei LOW/HIGH jeweils ohne äußere Flanke. Normalisierung `m_b=w_b/max(1,sum(w))`. Unzugeordnete Anteile bleiben im Originalsignal.

`sum = delayed_input + sum_b(wet_b * (processed_band_b - delayed_band_b))`.

Bei neutralen Parametern und unterhalb der Limitergrenze wird das um 1184 Samples verzögerte Eingangssignal rekonstruiert. Separate +6-dB-Überlappungstests prüfen die WOLA-Synthese, nicht nur den neutralen Differenzpfad. Fenster bleiben durch die endliche FFT-Auflösung begrenzt: 46,875 Hz/Bin bei 48 kHz, 93,75 Hz/Bin bei 96 kHz. Reelle symmetrische Masken verändern keine einzelne Bin-Phase, können aber Transienten zeitlich verteilen. Der unabhängige Cut wird nicht mehr aus diesen groben Masken abgeleitet.

## Oversampling

Jede Bandsättigung und GLUE besitzt eine feste 4x-Interpolation/Dezimation mit 65-Tap-FIR (Blackman-Fenster, normierter Cutoff 0,1125 der hohen Samplerate). Interpolation phasenweise, Skalierung 4; Dezimation einmal je vier Samples. Kumulierte Gruppenlaufzeit 64 Hochraten-Samples =16 Basissamples. Koeffizienten/History werden vor Audiobetrieb angelegt, kein Umschalten der Rate im Callback.

Nur die nichtlineare Differenz wird durch die FIR-Kette geschickt und zur entsprechend verzögerten linearen Referenz addiert. Damit bleibt Colour=0 exakt neutral, ohne einen unnötigen FIR-Frequenzgang auf dem trockenen Signal. Kurven: begrenzte rationale Tanh-Annäherung `x*(27+x*x)/(27+9*x*x)` für |x|≤3, darüber ±1; unterschiedliche Skalierung für VELVET/IRON. Kleinsignalsteigung 1. Der FIR ist endlich: Oversampling reduziert Aliasing, beseitigt es nicht unter allen Bedingungen vollständig.

Band-Originale, Summenreferenz und Global Bypass sind auf die 16+16 Samples ausgerichtet. IIR-Cuts haben frequenzabhängige Phase, aber keine zusätzliche feste Blocklatenz. Cut-Mix zwischen trocken und IIR-gefiltert erzeugt bei Zwischenwerten eine absichtliche phasenabhängige Mischantwort; nur Mix=1 ist der kalibrierte vollständige Butterworth-Cut.

## Kalibrierte Cuts

Getrennte, doppelt präzise RBJ-Biquad-Kaskaden. Butterworth-Ordnungen 2,4,8,16; Pol-Q `1/(2*cos(pi*(2k+1)/(2*order)))`. Vorverzerrung durch bilineare Koeffizienten. Gemeinsamer −3,0103-dB-Punkt bei der eingestellten Frequenz, begrenzt auf 10…0,45 Fs. Asymptotische analoge Flanken 12/24/48/96 dB/Okt.; nahe Nyquist gilt die bilinear verzerrte digitale Frequenzachse.

Alle vier Kaskaden laufen parallel; die Auswahl wird über Gewichte geglättet. Cut-Frequenzen folgen geglätteten Parametern, Koeffizienten werden alle acht Kontrollschritte aktualisiert. Keine Block-Lookahead-Latenz, kein Brickwall-Versprechen.

## Kontrollreihenfolge / Schleifenfreiheit

Ein Kontrollschritt pro 32 Samples:

1. Seedänderung und samplegetaktete Pong-Physik (außer HOLD).
2. Hysteretischen Stresszustand und adaptive Rücknahme fortschreiben.
3. Quellen der **vorherigen** Kontrollperiode durch die Routenglättung führen.
4. Begrenzte Modulationssummen bilden; Basiseinstellungen plus Bewegung/Matrix minus adaptive Eingriffe ergeben effektive Parameter.
5. Erst danach neue Band-/Transienten-/Sidechain-/MIDI-/Hallquellen für den nächsten Schritt übernehmen.

Damit sind gegenseitige Steuerwege zeitverzögert, nicht algebraisch. Zielparameter bleiben in festen Grenzen. Routenglättung mindestens 5 ms, Basiseinstellungen 20 ms. Wechsel von Zielmodul/Zielparameter werden über 30-ms-Routinggewichte überblendet, die ebenfalls im State gespeichert sind. MIDI-Events werden an ihrer Sampleposition gelesen, ihre Matrixwirkung folgt im Kontrollraster plus Routenglättung. Hostparameter werden blockweise übernommen, keine Behauptung samplegenauer Hostautomation innerhalb eines JUCE-Blocks.

ADAPT: Aktivierung bei Bandpeak >0,8 FS oder GR >12 dB; Entspannung erst unter 0,5 FS und GR <6 dB. Eingriffszeit 80 ms mit zusätzlicher Ratenbegrenzung; Erholung nach RECOVERY. Eingriff bis −24 dB Drive, Modulation bis 75 % reduziert. HOLD hält physikalische und Reglerzustände, nicht das Audio an.

## Hall, GLUE, Limiter

Zwei achtkanalige Householder-FDNs mit vier vorgeschalteten Allpass-Diffusoren und variabler Vorverzögerung. Feedback aus gewünschter nomineller −60-dB-Abklingzeit, Damping je Delayline. Unterschiedliche Delaylängen, zwei Stereoauslesungen. Decay ist ein nomineller Parameter, kein über alle Frequenzen exakt gleiches gemessenes RT60. PRE und POST-Instanz laufen ständig, ihre Routinggewichte werden geglättet.

GLUE: Peak-Hüllkurve, festes weiches Kompressionsverhältnis (dB-Steigung 0,25), höchstens 6 dB Reduktion × Amount, 150-ms-Peakerholung. Danach 4x-Sättigung und Trim.

Limiter: 129-Sample-Peakfenster und 128-Sample-Delay, gemeinsamer Stereo-Gain, unmittelbare begrenzende Regelung und 60-ms-Erholung. Letzte Samplebegrenzung schützt gegen Rundungsüberschreitungen. Keine True-Peak-Erkennung. Auch der globale Bypass führt durch diesen Schutz. Sehr starke Eingriffe sind hörbar; der Schutz wird nicht als transparentes Mastering-Limiting beworben.

## Realtime, State und Grenzen

Feste Arrays für DSP, FFT, FIR, FDN, Notentabelle und Limiter. Keine Heap-Allokation oder blockierenden Locks im DSP-Callback. MIDI wird direkt aus den Hostbytes gelesen; SysEx wird ignoriert, nicht kopiert. Eingänge NaN/Inf →0, extrem endliche Eingänge auf ±16 begrenzt. Ausgang-Finite-Check vor Limiter, Denormal-Schutz im JUCE-Callback.

GUI-Meter und Zustandssnapshots verwenden Atomics. Der Audiothread wartet nie auf die GUI. State-Snapshots mit Sequenzzähler enthalten Motion, Kontrollphase, Matrixwerte, Quellen, adaptive Zustände und RNG. Die Leseseite wiederholt bei einem gleichzeitigen Update begrenzt; Hall-/Audio-Delayhistory ist bewusst nicht serialisiert. Keine versteckte DAW-/Dateioperation im Callback.

Verbleibende Produktgrenzen: kein frei verkabelbares Audio-DAG, keine Aufwärtskompression/Expansion, kein MPE/Sustainpedal, kein True-Peak-Limiter und keine geprüfte Hardwareemulation. Die hier angeforderten PLAY-, Matrix-, Sidechain/MIDI-, Hall-, GLUE-, 4x-OS-, Cut- und Limiterfunktionen sind implementiert und keine Platzhalter.
