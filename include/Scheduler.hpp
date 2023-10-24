#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <Arduino.h>
#include <vector>
#include <functional>

class Scheduler
{
public:
    struct Task
    {
        const char *name;
        std::function<void()> function;
        unsigned long interval;
        unsigned long last_execution_time;
    };

    void add_task(
        const char *name,
        std::function<void()> function, unsigned long interval);
    void tick();

private:
    std::vector<Task> tasks;
};

#endif
