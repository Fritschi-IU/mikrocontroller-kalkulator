#Mikrocontroller-Kalkulator - Aufgae 2

Diese Repostory enthält die Umsetzung der Aufgabe 2 aus dem Kurs "Programmierung mit C/C++".
Thema: Mikrocontroller-Implementierung eines einfachen Kalkulators

#Projektbeschreibung

Ziel des Projekts ist die Umsetzung eines einfachen Klakulators, bei dem eine PC-Anwendung einen Rechenausdruck über eine serielle Schnittstelle an einen Mikrokontroller sendet.
Der Nutzer gibt in der PC-Anwendung einen Ausdruck ein, zum Beispiel 34*72. Dieser Ausdruck wird über den COM-Port an den Mikrocontroller übertragen. Der Mikrocontroller zerlegt die Eingabe in zwei Operanden und eine Operator, prüft die Eingabe, führt die Berechnung aus und sendet anschließend entweder das Ergebnis oder eine Fehlermeldung zurück.
Beispiel:

Eingabe: 34*72
Antwort: ERGEBNIS: 2448

##Projektstruktur 

main.c - Mikrocontroller-Anwendung in C für den ATmega328P / Arduino Uno
main.cpp - PC-Anwendung in C++ für Windows / Visual Studio
README.md - Projektbeschreibung und Hinweis zur Ausführung

##Mikrocontroller-Anwendung

Datei: main.c
Die Mikrocontroller-Anwendung ist in C geschreiben und für den ATmega328P des Arduino Uno vorgesehen.
Funktionen:

-Initialisierung der UART-Schnittstelle mit 9600 Baud
-Empfang einer vollständigen Eingabezeile
Zerlegung der Eingabe in Operand 1, Operator und Operand 2
-Prüfung auf gültige Eingabe
-Berechnung der Grundrechenarten +, -, * und /
-Rückgabe von Ergebnis oder Fehlermeldung über UART

Der Code verwendet keine Arduino-Serial-Klasse und keine Arduino-setup()-/loop()-Struktur, sondern arbeitet mit main(void) und direktem UART-Registerzugriff.

##PC-Anwendung
