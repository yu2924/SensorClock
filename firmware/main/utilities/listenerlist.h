//
//  listenerlist.h
//  SensorClock
//
//  created by yu2924 on 2026-03-06
//

#pragma once

#include <algorithm>
#include <list>

template<typename ListenerT> struct ListenerListT
{
    std::list<ListenerT*> listenerList;
    void add(ListenerT* p)
    {
        auto it = std::find(listenerList.begin(), listenerList.end(), p);
        if(it == listenerList.end()) { listenerList.push_back(p); }
    }
    void remove(ListenerT* p)
    {
        auto it = std::find(listenerList.begin(), listenerList.end(), p);
        if(it != listenerList.end()) { listenerList.erase(it); }
    }
    template<typename... EventArgs, typename... Args> void call(void (ListenerT::* eventmethod)(EventArgs...), Args&&... args) const
    {
        auto it = listenerList.begin();
        while(it != listenerList.end())
        {
            ListenerT* p = *it++;
            (p->*eventmethod)(args...);
        }
    }
};
