#include <iostream>
#include <vector>
using namespace std;

/**
 * This is a really limited observer pattern.
 * It's not fully implemented on purpose. For now,
 * Ther's no need for more advanced functions.
 */

template<typename T> class Observer;

template <typename T>
class Subject {
    vector < class Observer<T> * > views;

  public:
    void attach(Observer<T> *obs) {
        views.push_back(obs);
    };

    void notify(T usr_data) {
        for(Observer<T>* obs : views) 
            obs->update(usr_data);
    };
};

template <typename T>
class Observer {
public:
    virtual void update(T) = 0;
};