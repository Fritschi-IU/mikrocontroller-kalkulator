# Mikrocontroller-Kalkulator - Aufgabe 2

Diese Repostory enthält die Umsetzung der Aufgabe 2 aus dem Kurs "Programmierung mit C/C++".
Thema: Mikrocontroller-Implementierung eines einfachen Kalkulators

# Projektbeschreibung

Ziel des Projekts ist die Umsetzung eines einfachen Kalkulators, bei dem eine PC-Anwendung einen Rechenausdruck über eine serielle Schnittstelle an einen Mikrocontroller sendet.
Der Nutzer gibt in der PC-Anwendung einen Ausdruck ein, zum Beispiel 34*72. Dieser Ausdruck wird über den COM-Port an den Mikrocontroller übertragen. Der Mikrocontroller zerlegt die Eingabe in zwei Operanden und einen Operator, prüft die Eingabe, führt die Berechnung aus und sendet anschließend entweder das Ergebnis oder eine Fehlermeldung zurück.
Beispiel:

Eingabe: 34*72
Antwort: ERGEBNIS: 2448

## Projektstruktur 

main.c - Mikrocontroller-Anwendung in C für den ATmega328P / Arduino Uno
main.cpp - PC-Anwendung in C++ für Windows / Visual Studio
README.md - Projektbeschreibung und Hinweis zur Ausführung

## Mikrocontroller-Anwendung

Datei: main.c
Die Mikrocontroller-Anwendung ist in C geschrieben und für den ATmega328P des Arduino Uno vorgesehen.
Funktionen:

- Initialisierung der UART-Schnittstelle mit 9600 Baud
- Empfang einer vollständigen Eingabezeile
- Zerlegung der Eingabe in Operand 1, Operator und Operand 2
- Prüfung auf gültige Eingabe
- Berechnung der Grundrechenarten +, -, * und /
- Rückgabe von Ergebnis oder Fehlermeldung über UART

Der Code verwendet keine Arduino-Serial-Klasse und keine Arduino-setup()-/loop()-Struktur, sondern arbeitet mit main(void) und direktem UART-Registerzugriff.

## PC-Anwendung

Datei: main.cpp
Die PC-Anwendung ist als C++ Konsolenprogramm für Windows und Visual Studio umgesetzt.
Funktionen:

- Eingabe des COM-Ports
- Eingabe von Rechenausdrücken
- Senden der Eingabe an den Mikrocontroller
- Empfangen der Antwort
- Ausgabe im Konsolenfenster
- Protokollierung der Kommunikation in kommunikation_log.txt

Die serielle Kommunikation wird unter Windows über CreateFileA(), ReadFile() und WriteFile() umgesetzt.

## Unterstützte Rechenoperationen

Es werden zwei Operanden und ein Operator unterstützt.
Beispiel:

- 34*72
- 10/2
- 5-9
- 1/4
- 1-1.5

Unterstützte Operatoren

- Addition: +
- Subtraktion: -
- Multiplikation: *
- Division: /

Nicht unterstützt werden:

- Klammern
- mehrere Operatoren in einem Ausdruck
- wissenschaftliche Funktionen
- Operatorrangfolge bei mehreren Rechenschritten
  
Diese Einschränkungen entsprechen der Projektabgrenzung aus der Konzeptionsphase.

## Kompilierung der PC-Anwendung

Die PC-Anwendung kann in Visual Studio als C++ Konsolenprojekt ausgeführt werden.
Vorgehen:

1. Neues C++ Konsolenprojekt in Visual Studio erstellen
2. Automatisch erzeugte Quelldatei durch main.cpp ersetzen
3. Projekt mit "Lokaler Windows-Debugger" starten
4. COM-Port des Arduino Uno eingeben, zum Beispiel COM3
5. Rechenausdrücke im Konsolenfenster eingeben

## Kompilierung und Upload der Mikrocontroller-Anwendung 

Die Mikrocontroller-Anwendung wird mit avr-gcc kompiliert und mit avrdude auf dem Arduino Uno übertragen.
Beispielhafte PowerShell-Befehle:

```powershell
$AVRGCC = "$env:LOCALAPPDATA\Arduino15\packages\arduino\tools\avr-gcc\7.3.0-atmel3.6.1-arduino7\bin"

$AVRDUDE = "$env:LOCALAPPDATA\Arduino15\packages\arduino\tools\avrdude\8.0.0-arduino1"
```
Kompilierung:
```powershell
& "$AVRGCC\avr-gcc.exe" -mmcu=atmega328p -DF_CPU=16000000UL -Os -o main.elf main.c
```
HEX-Datei erzeugen:
```powershell
& "$AVRGCC\avr-objcopy.exe" -O ihex -R .eeprom main.elf main.hex
```
Upload auf dem Arduino Uno, Beispiel mit COM3:
```powershell
& "$AVRDUDE\bin\avrdude.exe" -C "$AVRDUDE\etc\avrdude.conf" -v -patmega328p -carduino -PCOM3 -b115200 -D -Uflash:w:main.hex:i
```
Hinweis: Der COM-Port muss bei Bedarf angepasst werden.
Während des Uploads darf kein serieller Monitor geöffnet sein, da dieser den COM-Port blockieren kann.

## Bedienung 

Nach dem Start der PC-Anwendung 
COM-Port eingeben: COM3

Danach können Rechenausdrücke eingegeben werden:
Eingabe: 34*72
Antwort: ERGEBNIS: 2448

Zum Beenden:
exit

## Logdatei

Die PC-Anwendung erzeugt automatisch die Datei kommunikation_log.txt.
Darin werden gesendete Ausdrücke und empfangene Antworten mit Zeitstempel gespeichert.
Beispiel:

[2026-06-04 18:30:10] SYSTEM: Programm gestartet, Port: COM3  
[2026-06-04 18:30:15] PC -> UC: 34 * 72  
[2026-06-04 18:30:15] UC -> PC: ERGEBNIS: 2448  
[2026-06-04 18:30:22] PC -> UC: 10 / 0  
[2026-06-04 18:30:22] UC -> PC: FEHLER: Division durch null

## Autor

Fritsch Lukas
Matrikelnummer: IU14147385
Kurs: DLBROEPRS01_D
