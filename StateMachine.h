//
// Created by Rahul  Kushwaha on 11/24/22.
//

#ifndef LOGSTORAGE_STATEMACHINE_H
#define LOGSTORAGE_STATEMACHINE_H
#include <string>

template<typename T>
class StateMachine {
 public:
  virtual void apply(const T &t) = 0;
};


#endif //LOGSTORAGE_STATEMACHINE_H
