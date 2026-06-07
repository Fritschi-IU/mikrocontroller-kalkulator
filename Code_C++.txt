/*
  Aufgabe 2 - PC-Anwendung
  C++-Konsolenprogramm fuer Windows / Visual Studio

  Funktion:
  - Benutzer gibt COM-Port ein, z. B. COM3
  - Benutzer gibt Rechenausdruck ein, z. B. 34 * 72
  - Ausdruck wird an den Mikrocontroller gesendet
  - Antwort wird im Konsolenfenster angezeigt
  - Gesamte Kommunikation wird in kommunikation_log.txt gespeichert
*/

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

std::string currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
    localtime_s(&localTime, &time);

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void writeLog(std::ofstream& logFile,
    const std::string& direction,
    const std::string& text) {
    logFile << "[" << currentTimestamp() << "] "
        << direction << ": " << text << std::endl;
}

void printWindowsError(const std::string& message) {
    DWORD errorCode = GetLastError();

    std::cout << message
        << " Windows-Fehlercode: "
        << errorCode
        << std::endl;
}

std::string buildPortName(const std::string& portName) {
    const std::string prefix = "\\\\.\\";

    if (portName.rfind(prefix, 0) == 0) {
        return portName;
    }

    return prefix + portName;
}

HANDLE openSerialPort(const std::string& portName) {
    std::string fullPortName = buildPortName(portName);

    HANDLE serialHandle = CreateFileA(
        fullPortName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (serialHandle == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();

        if (errorCode == ERROR_FILE_NOT_FOUND) {
            std::cout << "Port nicht gefunden. Pruefe den COM-Port-Namen."
                << std::endl;
        }
        else if (errorCode == ERROR_ACCESS_DENIED) {
            std::cout << "Zugriff verweigert. Port bereits in Verwendung "
                << "(serieller Monitor noch geoeffnet?)."
                << std::endl;
        }
        else {
            std::cout << "Fehler beim Oeffnen des Ports. Windows-Fehlercode: "
                << errorCode
                << std::endl;
        }

        return INVALID_HANDLE_VALUE;
    }

    DCB dcbSerialParams{};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(serialHandle, &dcbSerialParams)) {
        printWindowsError("FEHLER: Port-Einstellungen konnten nicht gelesen werden.");
        CloseHandle(serialHandle);
        return INVALID_HANDLE_VALUE;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fBinary = TRUE;
    dcbSerialParams.fParity = FALSE;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(serialHandle, &dcbSerialParams)) {
        printWindowsError("FEHLER: Port-Einstellungen konnten nicht gesetzt werden.");
        CloseHandle(serialHandle);
        return INVALID_HANDLE_VALUE;
    }

    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(serialHandle, &timeouts)) {
        printWindowsError("FEHLER: Timeouts konnten nicht gesetzt werden.");
        CloseHandle(serialHandle);
        return INVALID_HANDLE_VALUE;
    }

    PurgeComm(serialHandle, PURGE_RXCLEAR | PURGE_TXCLEAR);

    return serialHandle;
}

bool sendText(HANDLE serialHandle, const std::string& text) {
    DWORD bytesWritten = 0;
    DWORD bytesToWrite = static_cast<DWORD>(text.size());

    BOOL success = WriteFile(
        serialHandle,
        text.c_str(),
        bytesToWrite,
        &bytesWritten,
        nullptr
    );

    return success && bytesWritten == bytesToWrite;
}

std::string readLine(HANDLE serialHandle, unsigned long timeoutMs) {
    std::string result;
    char character = '\0';
    DWORD bytesRead = 0;

    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - startTime
        ).count();

        if (elapsedMs >= timeoutMs) {
            break;
        }

        BOOL success = ReadFile(
            serialHandle,
            &character,
            1,
            &bytesRead,
            nullptr
        );

        if (success && bytesRead > 0) {
            if (character == '\r') {
                continue;
            }

            if (character == '\n') {
                if (!result.empty()) {
                    break;
                }
            }
            else {
                result += character;
            }
        }
        else {
            Sleep(10);
        }
    }

    return result;
}

int main() {
    std::string portName;
    std::string input;

    std::cout << "PC-Anwendung: Mikrocontroller-Kalkulator" << std::endl;
    std::cout << "Beispiel COM-Port: COM3, COM4 oder COM5" << std::endl;
    std::cout << "COM-Port eingeben: ";
    std::getline(std::cin, portName);

    HANDLE serialHandle = openSerialPort(portName);

    if (serialHandle == INVALID_HANDLE_VALUE) {
        return 1;
    }

    std::ofstream logFile("kommunikation_log.txt", std::ios::app);

    if (!logFile.is_open()) {
        std::cout << "FEHLER: Logdatei konnte nicht geoeffnet werden."
            << std::endl;

        CloseHandle(serialHandle);
        return 1;
    }

    std::cout << "Verbindung hergestellt." << std::endl;
    std::cout << "Rechenausdruck eingeben, z. B. 34 * 72" << std::endl;
    std::cout << "Zum Beenden: exit" << std::endl << std::endl;

    writeLog(logFile, "SYSTEM", "Programm gestartet, Port: " + portName);

    /*
      Der Arduino Uno resettet sich automatisch, wenn der COM-Port geoeffnet wird.
      Der Bootloader benoetigt kurz Zeit. Ohne diese Wartezeit koennen die ersten
      gesendeten Bytes verloren gehen.
    */
    Sleep(1500);

    /* Startmeldungen des Mikrocontrollers empfangen und verwerfen */
    while (!readLine(serialHandle, 200).empty()) {
    }

    while (true) {
        std::cout << "Eingabe: ";
        std::getline(std::cin, input);

        if (input == "exit" || input == "EXIT") {
            break;
        }

        std::string message = input + "\n";

        if (!sendText(serialHandle, message)) {
            std::cout << "FEHLER: Ausdruck konnte nicht gesendet werden."
                << std::endl;

            writeLog(logFile, "FEHLER", "Senden fehlgeschlagen: " + input);
            continue;
        }

        writeLog(logFile, "PC -> UC", input);

        std::string response = readLine(serialHandle, 3000);

        if (response.empty()) {
            std::cout << "Keine Antwort vom Mikrocontroller erhalten."
                << std::endl;

            writeLog(logFile, "FEHLER", "Keine Antwort vom Mikrocontroller");
        }
        else {
            std::cout << "Antwort: " << response << std::endl;
            writeLog(logFile, "UC -> PC", response);
        }

        std::cout << std::endl;
    }

    writeLog(logFile, "SYSTEM", "Programm beendet");

    CloseHandle(serialHandle);

    std::cout << "Programm beendet." << std::endl;
    return 0;
}