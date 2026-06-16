#ifndef RETURN_H
#define RETURN_H

#include <any>

class ReturnValue {
public:
 explicit ReturnValue(std::any value) : value(std::move(value)) {};
 const std::any value;
};

#endif
