#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <vector>

constexpr double PING_TIMEOUT_SECONDS = 20;

class Controller
{
    public:
        Controller();
        void setup(void);
        void loop(void);

    public:
    private:
    private:
        unsigned long lastPingTime = 0;
        const char *TAG = "Controller";
};

extern Controller controller;

#endif // CONTROLLER_H