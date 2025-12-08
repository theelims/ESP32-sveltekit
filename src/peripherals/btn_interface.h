#ifndef BTN_INTERFACE_H
#define BTN_INTERFACE_H

#include <Button2.h>

class BtnInterface 
{
public:
    // Get the singleton instance
    static BtnInterface& getInstance();

    // Delete copy constructor and assignment operator to prevent copying
    BtnInterface(const BtnInterface&) = delete;
    BtnInterface& operator=(const BtnInterface&) = delete;

    // Initialize the button interface
    void initialize(int pin);

    // Update the button state (should be called in the loop)
    void update();

private:
    // Private constructor for Singleton
    BtnInterface();

    // Private destructor
    ~BtnInterface() = default;

    // Button2 object
    Button2 button;

    // Callback functions for button events
    static void onPress(Button2& btn);
    static void onRelease(Button2& btn);
    static void onChanged(Button2& btn);
    static void onClick(Button2& btn);
    static void onLongClickDetected(Button2& btn);
    static void onLongClick(Button2& btn);
    static void onDoubleClick(Button2& btn);
    static void onTripleClick(Button2& btn);
    static void onTap(Button2& btn);
};

#endif // BTN_INTERFACE_H