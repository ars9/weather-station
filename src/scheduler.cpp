#include "scheduler.hpp"

void Scheduler::add_task(const char *name, std::function<void()> function, unsigned long interval)
{
    Serial.printf("[Scheduler] Adding task \"%s\" with interval %dms\n", name, interval);
    Task task = {name, function, interval, 0};
    tasks.push_back(task);
}

void Scheduler::tick()
{
    unsigned long now = millis();

    for (auto &task : tasks)
    {
        if (now - task.last_execution_time >= task.interval)
        {
            task.function();
            task.last_execution_time = now;
        }
    }
}
