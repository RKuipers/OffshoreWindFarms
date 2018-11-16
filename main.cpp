#include <iostream>
#include "main.h"
#include "event.cpp"

using namespace std;

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
