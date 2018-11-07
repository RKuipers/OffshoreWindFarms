#include <vector>
#include <math.h>
#include "tasks.h"

using namespace std;

Task::Task() { }

Task::Task(int bD, int dl, vector<shared_ptr<Task>> prs, vector<shared_ptr<Task>> subs, vector<float> wE, int rT = 0) 
    : releaseTime(rT), deadline(dl), baseDuration(bD), status(0), timeCompleted(-1), prereqs(prs), subsequent(subs), weatherEffects(wE) { }

void Task::DoTask(int weatherCat, int t)
{
    if (t < releaseTime)
        return -1;

    if(!CheckAvailable())
        return -1;
    
    timeCompleted = t + ceil(baseDuration * weatherEffects[weatherCat]);
    nextActionTime = timeCompleted;
    status = 1;
}

vector<shared_ptr<Task>> Task::CompleteTask()
{
    nextActionTime = -1;
    status = 2;

    vector<shared_ptr<Task>> newlyAvailable;
    for(auto i = subsequent.begin(); i != subsequent.end(); i++)
    {
        if ((*i)->CheckAvailable())
            newlyAvailable.push_back(*i);
    }
    return newlyAvailable;
}

bool Task::CheckAvailable()
{
    for(auto i = prereqs.begin(); i != prereqs.end(); i++)
        if ((*i)->GetStatus() != 2)
            return false;

    return true;
}

int Task::GetNextActionTime()
{
    return nextActionTime;
}

int Task::GetStatus()
{
    return status;
}