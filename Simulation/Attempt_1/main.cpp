#include <iostream>
#include <vector>
#include "main.h"
#include "tasks.cpp"
#include "schedule.cpp"
#include "weather.cpp"
#include "vessel.cpp"

using namespace std;

bool CompareTasks::operator()(shared_ptr<Task> l, shared_ptr<Task> r)
{
    return l->GetNextActionTime() < r->GetNextActionTime();
}

int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}

Simulation::Simulation(vector<shared_ptr<Task>> tasksVec, shared_ptr<WeatherGen> wg, shared_ptr<Schedule> s)
    : t(0), tasks(tasksVec), weather(wg), schedule(s)
{
    //availableTasks.push_back(tasks[0]);
    queue.push(tasks[0]);
}

int Simulation::Run()
{
    while(queue.size() > 0)
    {
        shared_ptr<Task> task = queue.top();
        queue.pop();
        t = task->GetNextActionTime();

        if (task->GetStatus() == 0)
        {
            task->DoTask(0, t); // TODO: Actually give weather
        }
        else if (task->GetStatus() == 1)
        {
            vector<shared_ptr<Task>> newlyAvailable = task->CompleteTask();

            for (auto i = newlyAvailable.begin(); i != newlyAvailable.end(); i++)
            {
                queue.push(*i);
            }
        }
    }
}
