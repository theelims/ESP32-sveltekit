#include <Arduino.h>
#include <StrokeEngine.h>
#include <pattern.h>

void StrokeEngine::attachMotor(MotorInterface *motor)
{
  // store the machine geometry and motor properties pointer
  _motor = motor;

  // Initialize with default values
  _depth = _depthLimit = _strokeLimit = _motor->getMaxPosition();
  _stroke = constrain(MOTION_FACTORY_STROKE, 0.0, _depthLimit);
  _timeOfStroke = constrain(60.0 / MOTION_FACTORY_RATE, _timeOfStrokeLimit, 120.0);
  _sensation = MOTION_FACTORY_SENSATION;

  ESP_LOGD("StrokeEngine", "Stroke Parameter Depth = %f", _depth);
  ESP_LOGD("StrokeEngine", "Stroke Parameter Depth Limit = %f", _depthLimit);
  ESP_LOGD("StrokeEngine", "Stroke Parameter Stroke = %f", _stroke);
  ESP_LOGD("StrokeEngine", "Stroke Parameter Stroke Limit = %f", _strokeLimit);
  ESP_LOGD("StrokeEngine", "Stroke Parameter Time of Stroke = %f", _timeOfStroke);
  ESP_LOGD("StrokeEngine", "Stroke Parameter Sensation = %f", _sensation);

  ESP_LOGI("StrokeEngine", "Attached Motor successfully to Stroke Engine!");
}

float StrokeEngine::setParameter(StrokeParameter parameter, float value, bool applyNow)
{
  String name = "";
  float debugValue;
  if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
  {
    switch (parameter)
    {
    case StrokeParameter::RATE:
      name = "Stroke Time";
      // Convert FPM into seconds to complete a full stroke
      // Constrain stroke time between 100ms and 120 seconds
      _timeOfStroke = constrain(60.0 / value, _timeOfStrokeLimit, 120.0);
      debugValue = 60.0 / _timeOfStroke;
      break;

    case StrokeParameter::DEPTH:
      name = "Depth";
      debugValue = _depth = constrain(value, 0.0, _motor->getMaxPosition());
      break;

    case StrokeParameter::STROKE:
      name = "Stroke";
      debugValue = _stroke = constrain(value, 0.0, _motor->getMaxPosition());
      break;

    case StrokeParameter::SENSATION:
      name = "Sensation";
      debugValue = _sensation = constrain(value, -100.0, 100.0);
      break;
    }

    _sendParameters(_patternIndex);

    ESP_LOGD("StrokeEngine", "Stroke Parameter %s - %f", name, debugValue);

    // When running a pattern and immediate update requested:
    if (applyNow == true)
    {
      _applyUpdate = true;

      ESP_LOGD("StrokeEngine", "Setting Apply Update Flag!");
    }

    xSemaphoreGive(_parameterMutex);

    // return the actually used value after input sanitizing
    return debugValue;
  }

  // Add a default return statement
  return 0.0f;
}

float StrokeEngine::setLimit(StrokeLimit limit, float value, bool applyNow)
{
  String name = "";
  float debugValue;
  if (xSemaphoreTake(_parameterMutex, portMAX_DELAY) == pdTRUE)
  {
    switch (limit)
    {
    case StrokeLimit::RATE:
      name = "Rate";
      // Convert FPM into seconds to complete a full stroke
      // Constrain stroke time between 100ms and 120 seconds
      _timeOfStrokeLimit = constrain(60.0 / value, MIN_TIME_OF_STROKE, 120.0);
      // constrain current stroke time to new limit
      _timeOfStroke = constrain(_timeOfStroke, _timeOfStrokeLimit, 120.0);
      debugValue = 60.0 / _timeOfStrokeLimit;
      break;

    case StrokeLimit::DEPTH:
      name = "Depth";
      debugValue = _depthLimit = constrain(value, 0.0, _motor->getMaxPosition());
      // constrain current depth to new limit
      _depth = constrain(_depth, 0.0, _depthLimit);
      break;

    case StrokeLimit::STROKE:
      name = "Stroke";
      debugValue = _strokeLimit = constrain(value, 0.0, _motor->getMaxPosition());
      // constrain current stroke to new limit
      _stroke = constrain(_stroke, 0.0, _strokeLimit);
      break;
    }

    _sendParameters(_patternIndex);

    ESP_LOGD("StrokeEngine", "Stroke Limits %s - %f", name, debugValue);

    // When running a pattern and immediate update requested:
    if (applyNow == true)
    {
      _applyUpdate = true;

      ESP_LOGD("StrokeEngine", "Setting Apply Update Flag!");
    }

    xSemaphoreGive(_parameterMutex);

    return debugValue;
  }

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
    debugValue = 60.0 / _timeOfStrokeLimit;
    break;

  case StrokeLimit::DEPTH:
    name = "Depth";
    debugValue = _depthLimit;
    break;

  case StrokeLimit::STROKE:
    name = "Stroke";
    debugValue = _strokeLimit;
    break;
  }

  ESP_LOGD("StrokeEngine", "Get Stroke Limits %s - %f", name, debugValue);

  return debugValue;
}

// WARNING: This function must be called only within the scope of a Taken _parameterMutex
void StrokeEngine::_sendParameters(int patternIndex)
{
  patternTable[patternIndex]->setTimeOfStroke(_timeOfStroke);
  patternTable[patternIndex]->setStroke(_stroke);
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
      _sendParameters(_patternIndex);

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

float StrokeEngine::getParameter(StrokeParameter parameter)
{
  switch (parameter)
  {
  case StrokeParameter::RATE:
    return _timeOfStroke;
  case StrokeParameter::DEPTH:
    return _depth;
  case StrokeParameter::STROKE:
    return _stroke;
  case StrokeParameter::SENSATION:
    return _sensation;
  default:
    return 0; // Should never be reached
  }
}

int StrokeEngine::getCurrentPattern()
{
  return _patternIndex;
}

String StrokeEngine::getCurrentPatternName()
{
  return String(patternTable[_patternIndex]->getName());
}

bool StrokeEngine::startPattern()
{
  // Only valid if state is ready
  if (!_motor->isActive())
  {
    ESP_LOGE("StrokeEngine", "Failed to start pattern! Motor is not active!");
    return false;
  }

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
    _sendParameters(_patternIndex);
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

  return true;
}

void StrokeEngine::stopMotion()
{
  _active = false;
  _motor->stopMotion();
  ESP_LOGI("StrokeEngine", "Stopping Motion!");
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

void StrokeEngine::_stroking()
{
  motionParameter currentMotion;
  float targetPosition;

  // SemaphoreHandle_t semaphore = motor->claimMotorControl();

  // ESP_LOGD("StrokeEngine", "Hi, I am the pattern task.");

  while (1)
  { // infinite loop

    ESP_LOGV("StrokeEngine", "StrokeEngine is %s.", _active ? "active" : "not active");

    // Check if motor is still available
    if (_motor->isActive() == false)
    {
      ESP_LOGW("StrokeEngine", "Motor is no longer active! Attempting to suspend pattern.");
      _active = false;
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

        // Constrain stroke to ensure it obeys to motion boundaries
        currentMotion.stroke = constrain(currentMotion.stroke, 0.0, _stroke);
        // Offset stroke by depth
        targetPosition = (_depth - _stroke) + currentMotion.stroke;

        // Increase deceleration if required to avoid crash
        if (_motor->getAcceleration() > currentMotion.acceleration)
        {

          ESP_LOGW("StrokeEngine", "Crash avoidance! Set Acceleration from %05.1f to %05.1f", currentMotion.acceleration, _motor->getAcceleration());
          currentMotion.acceleration = _motor->getAcceleration();
        }

        // Apply new trapezoidal motion profile to servo
        ESP_LOGI("StrokeEngine", "Stroking Index (UPDATE): %d @ %05.1f mm %05.1f mm/s and %05.1f mm/s^2", _index, currentMotion.stroke, currentMotion.speed, currentMotion.acceleration);
        _motor->goToPosition(
            targetPosition,
            currentMotion.speed,
            currentMotion.acceleration);
      }

      // If motor has stopped issue moveTo command to next position
      else if (_motor->motionCompleted() == true)
      {
        // Increment index for pattern
        _index++;

        // Query new set of pattern parameters
        currentMotion = patternTable[_patternIndex]->nextTarget(_index);

        // Constrain stroke to ensure it obeys to motion boundaries
        currentMotion.stroke = constrain(currentMotion.stroke, 0.0, _stroke);
        // Offset stroke by depth
        targetPosition = (_depth - _stroke) + currentMotion.stroke;

        // Pattern may introduce pauses between strokes
        if (currentMotion.skip == false)
        {
          ESP_LOGI("StrokeEngine", "Stroking Index (AT_TARGET): %d @ %05.1f mm %05.1f mm/s and %05.1f mm/s^2", _index, currentMotion.stroke, currentMotion.speed, currentMotion.acceleration);
          _motor->goToPosition(
              targetPosition,
              currentMotion.speed,
              currentMotion.acceleration);
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
