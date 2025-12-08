#include "btn_interface.h"
#include <Logger.h>

#define TAG "BTN_INTERFACE"

// Static method to get the singleton instance
BtnInterface& BtnInterface::getInstance() 
{
    static BtnInterface instance; // Guaranteed to be destroyed and instantiated on first use
    return instance;
}

// Private constructor
BtnInterface::BtnInterface() 
    : button(Button2()) // Initialize Button2 object
{

}

// Initialize the button interface
void BtnInterface::initialize(int pin) 
{
    // Attach the button to the specified pin
    button.begin(pin);
    // button.setLongClickTime(1000);
    // button.setDoubleClickTime(400);

    LOG_I(TAG, "getLongClickTime: %d ms", button.getLongClickTime());
    LOG_I(TAG, "getDoubleClickTime: %d ms", button.getDoubleClickTime());
    // Attach callbacks for button events
    button.setPressedHandler(onPress);
    button.setReleasedHandler(onRelease);
    // button.setChangedHandler(onChanged);
    button.setClickHandler(onClick);
    button.setLongClickDetectedHandler(onLongClickDetected);
    button.setLongClickHandler(onLongClick);
    button.setLongClickDetectedRetriggerable(false);
    
    button.setDoubleClickHandler(onDoubleClick);
    button.setTripleClickHandler(onTripleClick);
    // button.setTapHandler(onTap);
}

// Update the button state (should be called in the loop)
void BtnInterface::update() 
{
    button.loop(); // Update the Button2 object
}

// Callback for button press event
void BtnInterface::onPress(Button2& btn) 
{
    LOG_I(TAG, "Button pressed.");
}

// Callback for button release event
void BtnInterface::onRelease(Button2& btn) 
{
    LOG_I(TAG, "Button released.");
    LOG_I(TAG, "wasPressedFor: %d ms", btn.wasPressedFor());
}

// Callback for button state change
void BtnInterface::onChanged(Button2& btn) 
{
    LOG_I(TAG, "Button state changed.");
}

// Callback for single click event
void BtnInterface::onClick(Button2& btn) 
{
    LOG_I(TAG, "Button clicked.");
}

// Callback for long click detected event
void BtnInterface::onLongClickDetected(Button2& btn) 
{
    LOG_I(TAG, "Long click detected.");
}

// Callback for long click event
void BtnInterface::onLongClick(Button2& btn) 
{
    LOG_I(TAG, "Long click performed.");
}

// Callback for double click event
void BtnInterface::onDoubleClick(Button2& btn) 
{
    LOG_I(TAG, "Button double-clicked.");
}

// Callback for triple click event
void BtnInterface::onTripleClick(Button2& btn) 
{
    LOG_I(TAG, "Button triple-clicked.");
    LOG_I(TAG, "getNumberOfClicks: %d", btn.getNumberOfClicks());
}

// Callback for tap event
void BtnInterface::onTap(Button2& btn) 
{
    LOG_I(TAG, "Button tapped.");
}