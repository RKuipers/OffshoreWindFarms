#ifndef MAIN_H
#define MAIN_H

#include <memory>
#include <queue>
#include "event.h"

using namespace std;

class CompareEvents
{
public:
    bool operator() (shared_ptr<Event> l, shared_ptr<Event> r);
};

class Simulation
{
private:    
    priority_queue<shared_ptr<Event>, vector<shared_ptr<Event>>, CompareEvents> queue;      // Event queue of simulation

public:
    Simulation();

    int Run();
};

#endif