#ifndef EVENT_H
#define EVENT_H

// Abstract Event class
class Event
{
private:
    int timeStamp;

public:
    Event(int time);

    virtual void EventOccurs() = 0;

    int GetTime();
};

#endif