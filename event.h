#ifndef EVENT_H
#define EVENT_H

// Abstract Event class
class Event
{
protected:
    int timeStamp;

public:
    Event(int time);

    virtual void EventOccurs() = 0;

    int GetTime();
};

// Event class representing the start of a journey
class JourneyEvent: public Event
{
public:
    void EventOccurs() override;
};

#endif