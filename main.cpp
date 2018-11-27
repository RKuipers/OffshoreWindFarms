#include <iostream>
#include <memory>
#include "main.h"
#include "event.cpp"

using namespace std;

bool CompareEvents::operator()(shared_ptr<Event> l, shared_ptr<Event> r)
{
    return l->GetTime() < r->GetTime();
}

Simulation::Simulation()
{
    string input;
    cout << "Hello World!" << endl;
    cin >> input;
}

int main(int argc, char const *argv[])
{
    Simulation* sim = new Simulation();
    return 0;
}
