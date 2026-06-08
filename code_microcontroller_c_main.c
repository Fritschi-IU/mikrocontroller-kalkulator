/*
 * Aufgabe 2 - Mikrocontroller-Kalkulator
 * Mikrocontroller-Anwendung in C
 * Zielplattform: Arduino Uno / ATmega328P, 16 MHz
 *
 * Funktion:
 * - Empfang eines Rechenausdrucks ueber UART, z. B. "34 * 72"
 * - Zerlegung in Operand 1, Operator und Operand 2
 * - Pruefung der Eingabe auf Gueltigkeit
 * - Berechnung der vier Grundrechenarten (+, -, *, /)
 * - Rueckgabe von Ergebnis oder Fehlermeldung ueber UART
 */

#define F_CPU 16000000UL
#define BAUD  9600UL

#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1UL)

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#define INPUT_BUFFER_SIZE  80
#define OUTPUT_BUFFER_SIZE 40

typedef enum {
    CALC_OK = 0,
    CALC_EMPTY_INPUT,
    CALC_INVALID_FIRST_OPERAND,
    CALC_MISSING_OPERATOR,
    CALC_UNKNOWN_OPERATOR,
    CALC_INVALID_SECOND_OPERAND,
    CALC_EXTRA_CHARACTERS,
    CALC_DIVISION_BY_ZERO,
    CALC_NUMBER_OUT_OF_RANGE
} CalcStatus;

static void uart_init(void);
static void uart_send_char(char character);
static void uart_send_string(const char *text);
static char uart_receive_char(void);
static void uart_read_line(char *buffer, unsigned int maxLength);

static const char *skip_spaces(const char *text);
static int value_is_output_safe(double value);
static CalcStatus calculate_expression(const char *input, double *result);
static const char *status_to_text(CalcStatus status);
static void format_result(double value, char *buffer, unsigned int maxLength);

int main(void) {
    char inputBuffer[INPUT_BUFFER_SIZE];
    char outputBuffer[OUTPUT_BUFFER_SIZE];
    double result = 0.0;
    CalcStatus status;

    uart_init();

    _delay_ms(500);

    while (1) {
        uart_read_line(inputBuffer, INPUT_BUFFER_SIZE);

        status = calculate_expression(inputBuffer, &result);

        if (status == CALC_OK) {
            format_result(result, outputBuffer, OUTPUT_BUFFER_SIZE);
            uart_send_string("ERGEBNIS: ");
            uart_send_string(outputBuffer);
            uart_send_string("\r\n");
        } else {
            uart_send_string("FEHLER: ");
            uart_send_string(status_to_text(status));
            uart_send_string("\r\n");
        }
    }

    return 0;
}

static void uart_init(void) {
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)(UBRR_VALUE);

    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart_send_char(char character) {
    while (!(UCSR0A & (1 << UDRE0))) {
    }

    UDR0 = character;
}

static void uart_send_string(const char *text) {
    while (*text != '\0') {
        uart_send_char(*text);
        text++;
    }
}

static char uart_receive_char(void) {
    while (!(UCSR0A & (1 << RXC0))) {
    }

    return UDR0;
}

static void uart_read_line(char *buffer, unsigned int maxLength) {
    unsigned int index = 0;
    char character;
    static char previousLineEnding = '\0';

    if (maxLength == 0) {
        return;
    }

    while (1) {
        character = uart_receive_char();

        if ((previousLineEnding == '\r' && character == '\n') ||
            (previousLineEnding == '\n' && character == '\r')) {
            previousLineEnding = '\0';
            continue;
        }

        previousLineEnding = '\0';

        if (character == '\r' || character == '\n') {
            previousLineEnding = character;
            break;
        }

        if (index < (maxLength - 1)) {
            buffer[index] = character;
            index++;
        }
    }

    buffer[index] = '\0';
}

static const char *skip_spaces(const char *text) {
    while (*text != '\0' && isspace((unsigned char)*text)) {
        text++;
    }

    return text;
}

static int value_is_output_safe(double value) {
    return (value >= (double)LONG_MIN) && (value <= (double)LONG_MAX);
}

static CalcStatus calculate_expression(const char *input, double *result) {
    const char *position;
    char *endPointer;
    double operand1;
    double operand2;
    char operation;

    position = skip_spaces(input);

    if (*position == '\0') {
        return CALC_EMPTY_INPUT;
    }

    operand1 = strtod(position, &endPointer);

    if (endPointer == position) {
        return CALC_INVALID_FIRST_OPERAND;
    }

    if (!value_is_output_safe(operand1)) {
        return CALC_NUMBER_OUT_OF_RANGE;
    }

    position = skip_spaces(endPointer);

    if (*position == '\0') {
        return CALC_MISSING_OPERATOR;
    }

    operation = *position;

    if (operation != '+' && operation != '-' &&
        operation != '*' && operation != '/') {
        return CALC_UNKNOWN_OPERATOR;
    }

    position++;
    position = skip_spaces(position);

    operand2 = strtod(position, &endPointer);

    if (endPointer == position) {
        return CALC_INVALID_SECOND_OPERAND;
    }

    if (!value_is_output_safe(operand2)) {
        return CALC_NUMBER_OUT_OF_RANGE;
    }

    position = skip_spaces(endPointer);

    if (*position != '\0') {
        return CALC_EXTRA_CHARACTERS;
    }

    switch (operation) {
        case '+':
            *result = operand1 + operand2;
            break;

        case '-':
            *result = operand1 - operand2;
            break;

        case '*':
            *result = operand1 * operand2;
            break;

        case '/':
            if (operand2 == 0.0) {
                return CALC_DIVISION_BY_ZERO;
            }
            *result = operand1 / operand2;
            break;

        default:
            return CALC_UNKNOWN_OPERATOR;
    }

    if (!value_is_output_safe(*result)) {
        return CALC_NUMBER_OUT_OF_RANGE;
    }

    return CALC_OK;
}

static const char *status_to_text(CalcStatus status) {
    switch (status) {
        case CALC_EMPTY_INPUT:
            return "Leere Eingabe";

        case CALC_INVALID_FIRST_OPERAND:
            return "Erster Operand ungueltig";

        case CALC_MISSING_OPERATOR:
            return "Operator fehlt";

        case CALC_UNKNOWN_OPERATOR:
            return "Unbekannter Operator";

        case CALC_INVALID_SECOND_OPERAND:
            return "Zweiter Operand ungueltig";

        case CALC_EXTRA_CHARACTERS:
            return "Unerwartete Zeichen am Ende";

        case CALC_DIVISION_BY_ZERO:
            return "Division durch null";

        case CALC_NUMBER_OUT_OF_RANGE:
            return "Zahl ausserhalb des darstellbaren Bereichs";

        case CALC_OK:
        default:
            return "OK";
    }
}

static void format_result(double value, char *buffer, unsigned int maxLength) {
    long integerPart;
    double decimalPart;
    long decimals;

    if (maxLength == 0) {
        return;
    }

    if (!value_is_output_safe(value)) {
        snprintf(buffer, maxLength, "Wert zu gross");
        buffer[maxLength - 1] = '\0';
        return;
    }

    integerPart = (long)value;

    if (value == (double)integerPart) {
        snprintf(buffer, maxLength, "%ld", integerPart);
        buffer[maxLength - 1] = '\0';
        return;
    }

    decimalPart = value - (double)integerPart;

    if (decimalPart < 0.0) {
        decimalPart = -decimalPart;
    }

    decimals = (long)(decimalPart * 10000.0 + 0.5);

    if (decimals >= 10000) {
        if (value < 0.0) {
            integerPart--;
        } else {
            integerPart++;
        }

        decimals = 0;
    }

    if (value < 0.0 && integerPart == 0) {
        snprintf(buffer, maxLength, "-0.%04ld", decimals);
    } else {
        snprintf(buffer, maxLength, "%ld.%04ld", integerPart, decimals);
    }

    buffer[maxLength - 1] = '\0';
}
