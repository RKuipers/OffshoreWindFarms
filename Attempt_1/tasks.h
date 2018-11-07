#ifndef TASKS_H
#define TASKS_H

#include <memory>
#include <vector>
#include <queue>

using namespace std;

class Task
{
private:
    int id;             // Task ID

    int releaseTime;    // Time after which the task is available
    int deadline;       // Time at which the task should be completed
    int baseDuration;   // Time the task would take in perfect weather

    int status;         // 0 = unstarted, 1 = currently being worked on, 2 = completed
    int timeCompleted;  // Only gets assigned after completion

    vector<shared_ptr<Task>> prereqs;       // Tasks that need to be completed before this task can start
    vector<shared_ptr<Task>> subsequent;    // Tasks that require this task to be completed before they can start
    vector<float> weatherEffects;           // Per category of weather, the effect it has on the duration (1.5f means it takes 50% longer than the base duration)    

    int nextActionTime; // Either the time it's scheduled (if all prereqs are completed) or the time it's completed. Used in the event priority queue.

public:
    Task();
    Task(int bD, int dl, vector<shared_ptr<Task>> prs, vector<shared_ptr<Task>> subs, vector<float> wE, int rT);

    void DoTask(int weatherCat, int t);
    vector<shared_ptr<Task>> CompleteTask();

    bool CheckAvailable();

    int GetNextActionTime();
    int GetStatus();
};

#endif