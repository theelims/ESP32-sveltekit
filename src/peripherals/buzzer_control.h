#ifndef BUZZER_CONTROL_H
#define BUZZER_CONTROL_H

#include <Arduino.h>
#include <string>

#include "board_def.h"

class BuzzerControl 
{
public:
    // Get the singleton instance
    static BuzzerControl& getInstance();

    // Delete copy constructor and assignment operator to prevent copying
    BuzzerControl(const BuzzerControl&) = delete;
    BuzzerControl& operator=(const BuzzerControl&) = delete;

    void loop(void);
    void playTone(uint8_t tone);

private:
    // Private constructor for Singleton
    BuzzerControl();

    // Private destructor
    ~BuzzerControl() = default;

    void initBuzzer();

private:
    static constexpr int melodySize = 8; // Explicitly define the size of the melody array
    int melody[melodySize] = {262, 294, 330, 349, 392, 440, 494, 523}; // C D E F G A B C
    int noteDurations[melodySize] = {4, 4, 4, 4, 4, 4, 4, 4};          // quarter notes
    const char *TAG = "Beeper-Ctrl";
};

#endif