#ifndef MAIN_H
#define MAIN_H

#include "tasks.h"
#include "schedule.h"
#include "weather.h"

class CompareTasks
{
public:
    bool operator() (shared_ptr<Task> l, shared_ptr<Task> r);
};

class Simulation
{
private:
    int t;  // Current timestamp of the simulation

    vector<shared_ptr<Task>> tasks;                                                     // General inventory of all tasks
    //vector<shared_ptr<Task>> availableTasks;                                          // Set of tasks available at the current time // TODO: Remove if not necessary (cause queue is better)
    priority_queue<shared_ptr<Task>, vector<shared_ptr<Task>>, CompareTasks> queue;     // Event queue of simulation

    shared_ptr<WeatherGen> weather;     // Weather generation engine
    shared_ptr<Schedule> schedule;      // Schedule to try and keep to

public:
    Simulation(vector<shared_ptr<Task>> tasksVec, shared_ptr<WeatherGen> wg, shared_ptr<Schedule> s);

    int Run();
};

#endif