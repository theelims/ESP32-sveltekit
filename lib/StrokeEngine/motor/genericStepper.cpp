#include <Arduino.h>
#include <motor/genericStepper.h>
#include <FastAccelStepper.h>

FastAccelStepperEngine engine = FastAccelStepperEngine();

// Init gin66/FastAccelStepper
void GenericStepperMotor::begin(motorProperties *motor)
{
    _motor = motor;

    // Setup FastAccelStepper
    engine.init();
    _stepper = engine.stepperConnectToPin(_motor->stepPin);
    if (_stepper)
    {
        _stepper->setDirectionPin(_motor->directionPin, _motor->invertDirection);
        _stepper->setEnablePin(_motor->enablePin, _motor->enableActiveLow);
        _stepper->setAutoEnable(false);
        _stepper->disableOutputs();
        ESP_LOGI("GenericStepper", "FastAccelStepper Engine successfully initialized!");
    }
    else
    {
        ESP_LOGE("GenericStepper", "Failed to load FastAccelStepper Engine!");
    }
}

void GenericStepperMotor::setMachineGeometry(float travel, float keepout)
{
    _travel = travel;
    _keepout = keepout;
    _maxPosition = travel - (keepout * 2);
    _minStep = 0;
    _maxStep = int(0.5 + _maxPosition * _motor->stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _motor->stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _motor->stepsPerMillimeter);
    ESP_LOGD("GenericStepper", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("GenericStepper", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("GenericStepper", "Machine Geometry MaxPosition = %f", _maxPosition);
}

// Assumes always homing to back of machine for safety
void GenericStepperMotor::setSensoredHoming(int homePin = 0, uint8_t arduinoPinMode, bool activeLow)
{
    // set homing pin as input
    _homingPin = homePin;
    pinMode(_homingPin, arduinoPinMode);
    _homingActiveLow = activeLow;
    ESP_LOGI("GenericStepper", "Homing switch on pin %i in pin mode %i is %s", _homingPin, arduinoPinMode, _homingActiveLow ? "active low" : "active high");
}

void GenericStepperMotor::home(float homePosition, float speed)
{
    _homePosition = int(0.5 + homePosition / float(_motor->stepsPerMillimeter));
    _homingSpeed = speed * _motor->stepsPerMillimeter;
    ESP_LOGI("GenericStepper", "Search home with %05.1f mm/s at %05.1f mm.", speed, homePosition);

    // set homed to false so that isActive() becomes false
    _homed = false;

    // first stop current motion and suspend motion tasks
    stopMotion();

    // Quit if stepper not enabled
    if (_enabled == false)
    {
        ESP_LOGE("GenericStepper", "Homing not possible! --> Enable stepper first!");
        return;
    }

    // Create homing task
    xTaskCreatePinnedToCore(
        this->_homingProcedureImpl, // Function that should be called
        "Homing",                   // Name of the task (for debugging)
        4096,                       // Stack size (bytes)
        this,                       // Pass reference to this class instance
        1,                          // Pretty high task priority
        &_taskHomingHandle,         // Task handle
        1                           // Have it on application core
    );
    ESP_LOGD("GenericStepper", "Created Homing Task.");
}

void GenericStepperMotor::home(void (*callBackHoming)(bool), float homePosition, float speed)
{
    _callBackHoming = callBackHoming;
    home(homePosition, speed);
}

void GenericStepperMotor::attachPositionFeedback(void (*cbMotionPoint)(unsigned int, float, float), unsigned int timeInMs)
{
    _cbMotionPoint = cbMotionPoint;
    _timeSliceInMs = timeInMs / portTICK_PERIOD_MS;
}

// Control
void GenericStepperMotor::enable()
{
    ESP_LOGI("GenericStepper", "Stepper Enabled!");
    // Enable stepper
    _enabled = true;
    _stepper->enableOutputs();

    if (_cbMotionPoint == NULL)
    {
        ESP_LOGD("GenericStepper", "No Position Feedback Task created.");
        return;
    }

    // Create / resume motion feedback task
    if (_taskPositionFeedbackHandle == NULL)
    {
        // Create Stroke Task
        xTaskCreatePinnedToCore(
            _positionFeedbackTaskImpl,    // Function that should be called
            "Motion Simulation",          // Name of the task (for debugging)
            4096,                         // Stack size (bytes)
            this,                         // Pass reference to this class instance
            10,                           // Pretty high task priority
            &_taskPositionFeedbackHandle, // Task handle
            1                             // Pin to application core
        );
        ESP_LOGD("GenericStepper", "Created Position Feedback Task.");
    }
    else
    {
        // Resume task, if it already exists
        vTaskResume(_taskPositionFeedbackHandle);
        ESP_LOGD("GenericStepper", "Resumed Position Feedback Task.");
    }
}

void GenericStepperMotor::disable()
{
    ESP_LOGI("GenericStepper", "Stepper Disabled!");
    // Disable stepper
    _enabled = false;
    _stepper->disableOutputs();

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
        vTaskDelete(_taskHomingHandle);
        _taskHomingHandle = NULL;
        ESP_LOGD("GenericStepper", "Deleted Homing Task.");
    }

    // Suspend motion feedback task if it exists already
    if (_taskPositionFeedbackHandle != NULL)
    {
        vTaskSuspend(_taskPositionFeedbackHandle);
        ESP_LOGD("GenericStepper", "Suspended Position Feedback Task.");
    }
}

void GenericStepperMotor::stopMotion()
{

    ESP_LOGW("GenericStepper", "STOP MOTION!");

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
        vTaskDelete(_taskHomingHandle);
        _taskHomingHandle = NULL;
        ESP_LOGD("GenericStepper", "Deleted Homing Task.");
    }

    if (_stepper->isRunning())
    {
        // Stop servo motor as fast as legally allowed
        _stepper->setAcceleration(_maxStepAcceleration);
        _stepper->applySpeedAcceleration();
        _stepper->stopMove();
        ESP_LOGD("GenericStepper", "Bring stepper to a safe halt.");
    }

    // Wait for servo stopped
    while (_stepper->isRunning())
        ;
}

void GenericStepperMotor::_unsafeGoToPosition(float position, float speed, float acceleration)
{
    // Translate between metric and steps
    int speedInHz = int(0.5 + speed * _motor->stepsPerMillimeter);
    int stepAcceleration = int(0.5 + acceleration * _motor->stepsPerMillimeter);
    int positionInSteps = int(0.5 + position * _motor->stepsPerMillimeter);
    ESP_LOGD("GenericStepper", "Going to unsafe position %i steps @ %i steps/s, %i steps/s^2", positionInSteps, speedInHz, stepAcceleration);

    // write values to stepper
    _stepper->setSpeedInHz(speedInHz);
    _stepper->setAcceleration(stepAcceleration);
    _stepper->moveTo(positionInSteps);
}

bool GenericStepperMotor::_queryHome()
{
    return (digitalRead(_homingPin) == !_homingActiveLow) ? true : false;
}

void GenericStepperMotor::_homingProcedure()
{
    // Init homing
    _beginHoming();

    // Set feedrate for homing
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration / 10);

    // Check if we are already at the home position
    if (_queryHome())
    {
        ESP_LOGD("GenericStepper", "Already at home position. Backing up and try again.");
        // back off 2*keepout from switch
        _stepper->move(_motor->stepsPerMillimeter * 2 * _keepout);

        // wait for move to complete
        while (_stepper->isRunning())
        {
            // Pause the task for 100ms while waiting for move to complete
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        // move back towards endstop
        _stepper->move(-_motor->stepsPerMillimeter * 4 * _keepout);
    }
    else
    {
        ESP_LOGD("GenericStepper", "Start searching for home.");
        // Move maximum travel distance + 2*keepout towards the homing switch
        _stepper->move(-_motor->stepsPerMillimeter * (_maxPosition + 4 * _keepout));
    }

    // Poll homing switch
    while (_stepper->isRunning())
    {

        // Are we at the home position?
        if (_queryHome())
        {
            ESP_LOGD("GenericStepper", "Found home!");
            // Set home position
            // Switch is at -KEEPOUT
            _stepper->forceStopAndNewPosition(_motor->stepsPerMillimeter * int(_homePosition - _keepout));

            // Do what needs to be done at the home position
            _atHome();

            // drive free of switch and set axis to lower end
            _stepper->moveTo(_minStep);

            _homed = true;

            // drive free of switch and set axis to 0
            _stepper->moveTo(0);

            // Break loop, home was found
            break;
        }

        // Pause the task for 20ms to allow other tasks
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // disable Servo if homing has not found the homing switch
    if (!_homed)
    {
        _stepper->disableOutputs();
        ESP_LOGE("GenericStepper", "Homing failed! Did not find home position.");
    }

    // Call notification callback, if it was defined.
    if (_callBackHoming != NULL)
    {
        _callBackHoming(_homed);
    }

    // delete one-time task
    _taskHomingHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("GenericStepper", "Homing task self-terminated");
}

void GenericStepperMotor::_positionFeedbackTask()
{
    TickType_t xLastWakeTime;
    // Initialize the xLastWakeTime variable with the current tick count.
    xLastWakeTime = xTaskGetTickCount();

    unsigned int now = millis();

    while (true)
    {
        // Return results of current motion point via the callback
        _callBackMotionPoint();

        // Delay the task until the next tick count
        vTaskDelayUntil(&xLastWakeTime, _timeSliceInMs);
    }
}

void GenericStepperMotor::_callBackMotionPoint()
{
    // Return results of current motion point via the callback
    _cbMotionPoint(
        millis(),
        getPosition(),
        getSpeed());
}

// FastAccelStepperEngine &GenericStepperMotor::fastAccelStepperEngineReference()
// {
//     return engine;
// }
