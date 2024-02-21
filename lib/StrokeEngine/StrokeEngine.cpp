#include <Arduino.h>
#include <StrokeEngine.h>
#include <StrokeEnginePattern.h>

void StrokeEngine::attachMotor(MotorInterface *motor)
{
  // store the machine geometry and motor properties pointer
  _motor = motor;

  // Initialize the trapezoidal profile
  _trapezoidalProfile.resetProfileTo(0.0);

  // Initialize with default values
  _safeGuard.begin(_motor,
                   _motor->getMaxPosition(),
                   MOTION_FACTORY_STROKE,
                   MOTION_FACTORY_RATE,
                   _motor->getMaxPosition(),
                   _motor->getMaxPosition(),
                   MOTION_MAX_RATE,
                   MOTION_MAX_VELOCITY,
                   MOTION_FACTORY_EASE_IN_VELOCITY);

  _sensation = MOTION_FACTORY_SENSATION;

  _easeIn.begin();

  ESP_LOGD("StrokeEngine", "Stroke Parameter Sensation = %.2f", _sensation);
  ESP_LOGI("StrokeEngine", "Attached Motor successfully to Stroke Engine!");
}

bool StrokeEngine::runCommand(StrokeCommand command)
{
  // Catch all commands if motor is not active
  if (!_motor->isActive())
  {
    // Stop command is always allowed
    if (command == StrokeCommand::STOP)
    {
      _command = command;
      _active = false; // catch to be sure
      return true;
    }
    else
    {
      ESP_LOGE("StrokeEngine", "Failed to execute command! Motor is not active!");
      return false;
    }
  }

  // Store command as internal state and return true
  _command = command;

  // Process command
  switch (command)
  {
  case StrokeCommand::STOP:
    _stopMotion();
    break;

  case StrokeCommand::RETRACT:
    if (_active)
      _stopMotion();
    _updateFixedPosition();
    break;

  case StrokeCommand::DEPTH:
    if (_active)
      _stopMotion();
    _updateFixedPosition();
    break;

  case StrokeCommand::STROKE:
    if (_active)
      _stopMotion();
    _updateFixedPosition();
    break;

  case StrokeCommand::PATTERN:
    _startPattern();
    break;

  case StrokeCommand::STROKESTREAM:
    // Stop while streaming is not implemented yet
    if (_active)
      _stopMotion();
    _command = StrokeCommand::STOP;
    break;

  case StrokeCommand::POSITIONSTREAM:
    // Stop while streaming is not implemented yet
    if (_active)
      _stopMotion();
    _command = StrokeCommand::STOP;
    break;

  default:
    _stopMotion();
    _command = StrokeCommand::STOP;
    break;
  }

  return true;
}

StrokeCommand StrokeEngine::getCommand()
{
  return _command;
}

float StrokeEngine::setParameter(StrokeParameter parameter, float value)
{
  float sanitizedValue;
  if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
  {
    switch (parameter)
    {
    case StrokeParameter::RATE:
      sanitizedValue = _safeGuard.setRate(value);
      break;

    case StrokeParameter::DEPTH:
      sanitizedValue = _safeGuard.setDepth(value);
      _easeIn.setDepth(sanitizedValue);
      break;

    case StrokeParameter::STROKE:
      sanitizedValue = _safeGuard.setStroke(value);
      _easeIn.setStroke(sanitizedValue);
      break;

    case StrokeParameter::SENSATION:
      _sensation = constrain(value, -100.0, 100.0);
      ESP_LOGD("StrokeEngine", "Set Stroke Parameter Sensation = %.2f", _sensation);
      sanitizedValue = _sensation;
      break;
    }

    _sendParametersToPattern(_patternIndex);

    xSemaphoreGive(_parameterMutex);

    // return the actually used value after input sanitizing
    return sanitizedValue;
  }

  // Add a default return statement
  return 0.0f;
}

float StrokeEngine::getParameter(StrokeParameter parameter)
{
  switch (parameter)
  {
  case StrokeParameter::RATE:
    return _safeGuard.getRate();
  case StrokeParameter::DEPTH:
    return _safeGuard.getDepth();
  case StrokeParameter::STROKE:
    return _safeGuard.getStroke();
  case StrokeParameter::SENSATION:
    return _sensation;
  default:
    return 0; // Should never be reached
  }
}

float StrokeEngine::setLimit(StrokeLimit limit, float value)
{
  float sanitizedValue;
  if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
  {
    switch (limit)
    {
    case StrokeLimit::RATE:
      sanitizedValue = _safeGuard.setRateLimit(value);
      break;

    case StrokeLimit::VELOCITY:
      sanitizedValue = _safeGuard.setVelocityLimit(value);
      break;

    case StrokeLimit::DEPTH:
      sanitizedValue = _safeGuard.setDepthLimit(value);
      break;

    case StrokeLimit::STROKE:
      sanitizedValue = _safeGuard.setStrokeLimit(value);
      break;
    }

    _sendParametersToPattern(_patternIndex);

    // Give mutex back
    xSemaphoreGive(_parameterMutex);

    return sanitizedValue;
  }

  if (value != sanitizedValue)
    _notify("Parameter restricted");

  return 0.0f;
}

float StrokeEngine::getLimit(StrokeLimit limit)
{
  String name = "";
  float debugValue;

  switch (limit)
  {
  case StrokeLimit::RATE:
    name = "Rate";
    debugValue = _safeGuard.getRateLimit();
    break;

  case StrokeLimit::VELOCITY:
    name = "Velocity";
    debugValue = _safeGuard.getVelocityLimit();
    break;

  case StrokeLimit::DEPTH:
    name = "Depth";
    debugValue = _safeGuard.getDepthLimit();
    break;

  case StrokeLimit::STROKE:
    name = "Stroke";
    debugValue = _safeGuard.getStrokeLimit();
    break;
  }

  ESP_LOGD("StrokeEngine", "Get Stroke Limits %s - %f", name, debugValue);

  return debugValue;
}

float StrokeEngine::setEaseInVelocity(float value)
{
  float easeInVelocity = 0.0;
  if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
  {
    easeInVelocity = _safeGuard.setEaseInSpeed(value);
    _easeIn.setEaseInSpeed(easeInVelocity);
    xSemaphoreGive(_parameterMutex);
  }
  return easeInVelocity;
}

float StrokeEngine::getEaseInVelocity()
{
  return _safeGuard.getEaseInSpeed();
}

void StrokeEngine::applyChangesNow()
{
  if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
  {
    _applyUpdate = true;
    ESP_LOGD("StrokeEngine", "Setting Apply Update Flag!");
    xSemaphoreGive(_parameterMutex);
  }
}

// WARNING: This function must be called only within the scope of a Taken _parameterMutex
void StrokeEngine::_sendParametersToPattern(int patternIndex)
{
  patternTable[patternIndex]->setTimeOfStroke(_safeGuard.getTimeOfStroke());
  patternTable[patternIndex]->setStroke(_safeGuard.getStroke());
  patternTable[patternIndex]->setSensation(_sensation);
}

bool StrokeEngine::setPattern(int patternIndex, bool applyNow)
{
  // Check wether pattern Index is in range
  if ((patternIndex < patternTableSize) && (patternIndex >= 0))
  {
    if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
    {
      _patternIndex = patternIndex;

      // Inject current motion parameters into new pattern
      _sendParametersToPattern(_patternIndex);

      // Reset index counter
      _index = 0;
      xSemaphoreGive(_parameterMutex);
    }
    ESP_LOGD("StrokeEngine", "Load Pattern %i - %s", _patternIndex, getPatternName(_patternIndex).c_str());
    return true;
  }

  // Return false on no match
  ESP_LOGE("StrokeEngine", "Failed to set pattern!");
  return false;
}

bool StrokeEngine::setPattern(String patternName, bool applyNow)
{
  ESP_LOGD("StrokeEngine", "Select pattern by string: %s", patternName);
  for (size_t i = 0; i < patternTableSize; i++)
  {
    if (strcmp(patternTable[i]->getName(), patternName.c_str()) == 0)
    {
      setPattern(i, applyNow);
      return true;
    }
  }

  // Return false on no match
  ESP_LOGE("StrokeEngine", "Failed to set pattern!");
  return false;
}

int StrokeEngine::getCurrentPattern()
{
  return _patternIndex;
}

String StrokeEngine::getCurrentPatternName()
{
  return String(patternTable[_patternIndex]->getName());
}

String StrokeEngine::getPatternName(int index)
{
  if (index >= 0 && index <= patternTableSize)
  {
    return String(patternTable[index]->getName());
  }
  else
  {
    return String("Invalid");
  }
}

void StrokeEngine::onNotify(StrokeEngineNotifyCallback callback)
{
  _onNotifyCallbacks.push_back(callback);
}

void StrokeEngine::_notify(String message)
{
  for (auto &callback : _onNotifyCallbacks)
  {
    callback(message);
  }
}
void StrokeEngine::_updateFixedPosition()
{
  float target;

  switch (_command)
  {
  case StrokeCommand::RETRACT:
    target = 0.0;
    break;

  case StrokeCommand::DEPTH:
    target = _safeGuard.getDepth();
    break;

  case StrokeCommand::STROKE:
    target = _safeGuard.getDepth() - _safeGuard.getStroke();
    break;

  default:
    return;
  }

  float speed = _safeGuard.getEaseInSpeed();
  float acceleration = speed * 2.0;

  // Apply new trapezoidal motion profile to servo
  ESP_LOGI("StrokeEngine", "Fixed Position Move [%d] to: %05.1f mm @ %05.1f mm/s and %05.1f mm/s^2", _command, target, speed, acceleration);
  // _trapezoidalProfile.generateTrapezoidalProfile(target, speed, acceleration);
  _motor->goToPosition(target, speed, acceleration);
}

void StrokeEngine::_startPattern()
{
  Pattern *pattern = patternTable[_patternIndex];
  ESP_LOGI("StrokeEngine", "Starting pattern %s", pattern->getName());

  // Stop current move, should one be pending (moveToMax or moveToMin)
  if (_motor->motionCompleted() == false)
  {
    _motor->stopMotion();
  }

  // Reset Stroke and Motion parameters
  _index = -1;
  if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
  {
    _sendParametersToPattern(_patternIndex);
    xSemaphoreGive(_parameterMutex);
  }

  // Set active flag
  _active = true;

  if (_taskStrokingHandle == NULL)
  {
    // Create Stroke Task
    xTaskCreatePinnedToCore(
        _strokingImpl,        // Function that should be called
        "Stroking",           // Name of the task (for debugging)
        4096,                 // Stack size (bytes)
        this,                 // Pass reference to this class instance
        24,                   // Pretty high task priority
        &_taskStrokingHandle, // Task handle
        1                     // Pin to application core
    );
    ESP_LOGD("StrokeEngine", "Created Pattern Task.");
  }
  else
  {
    // Resume task, if it already exists
    vTaskResume(_taskStrokingHandle);
    ESP_LOGD("StrokeEngine", "Resumed Pattern Task.");
  }

  return;
}

void StrokeEngine::_stopMotion()
{
  ESP_LOGI("StrokeEngine", "Stopping Motion!");
  _active = false;
  _motor->stopMotion();
}

void StrokeEngine::_stroking()
{
  motionParameters_t currentMotion;
  SafeStrokeParameters_t safeMotion;
  float targetPosition;

  while (1)
  { // infinite loop

    // ESP_LOGV("StrokeEngine", "StrokeEngine is %s.", _active ? "active" : "not active");

    // Check if motor is still available
    if (_motor->isActive() == false)
    {
      ESP_LOGW("StrokeEngine", "Motor is no longer active! Attempting to suspend pattern.");
      _active = false;
      _command = StrokeCommand::STOP;
      _notify("Motor Error");
    }

    // Suspend task, if motor is not active
    if (_active == false)
    {
      vTaskSuspend(_taskStrokingHandle);
      ESP_LOGD("StrokeEngine", "Suspended Pattern Task.");
    }

    // ESP_LOGD("StrokeEngine", "Attempting to take the mutex in pattern task.");
    //  Take mutex to ensure no interference / race condition with communication threat on other core
    if (xSemaphoreTake(_parameterMutex, 0) == pdTRUE)
    {

      if (_applyUpdate == true && _motor->motionCompleted() == false)
      {
        // Ask pattern for update on motion parameters
        currentMotion = patternTable[_patternIndex]->nextTarget(_index);

        // Run safety system on new motion parameters
        safeMotion = _safeGuard.makeSafe(currentMotion);

        // Apply the ease-in modifier to the motion parameters
        // _easeIn.applyModification(safeMotion);

        // Apply new trapezoidal motion profile to servo
        ESP_LOGI("StrokeEngine", "Stroking Index (UPDATE): %d @ %05.1f mm %05.1f mm/s and %05.1f mm/s^2", _index, safeMotion.absoluteTargetPosition, safeMotion.speed, safeMotion.acceleration);
        _motor->goToPosition(
            safeMotion.absoluteTargetPosition,
            safeMotion.speed,
            safeMotion.acceleration);
      }

      // If motor has stopped issue moveTo command to next position
      else if (_motor->motionCompleted() == true)
      {
        // Increment index for pattern
        _index++;

        // Query new set of pattern parameters
        currentMotion = patternTable[_patternIndex]->nextTarget(_index);

        // Pattern may introduce pauses between strokes
        if (currentMotion.skip == false)
        {
          // Run safety system on new motion parameters
          safeMotion = _safeGuard.makeSafe(currentMotion);

          // Apply the ease-in modifier to the motion parameters
          _easeIn.applyModification(safeMotion);

          // Apply new trapezoidal motion profile to servo
          ESP_LOGI("StrokeEngine", "Stroking Index (AT_TARGET): %d @ %05.1f mm %05.1f mm/s and %05.1f mm/s^2", _index, safeMotion.absoluteTargetPosition, safeMotion.speed, safeMotion.acceleration);
          _motor->goToPosition(
              safeMotion.absoluteTargetPosition,
              safeMotion.speed,
              safeMotion.acceleration);
        }
        else
        {
          // decrement _index so that it stays the same until the next valid stroke parameters are delivered
          _index--;
        }
      }
      else
      {
        // ESP_LOGV("StrokeEngine", "Trapezoidal motion still pending");
      }

      // clear update flag, should be one pending
      _applyUpdate = false;

      // give back mutex
      xSemaphoreGive(_parameterMutex);
    }

    // Delay 10ms
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
