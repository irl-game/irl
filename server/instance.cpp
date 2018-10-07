#include "instance.hpp"

Instance::Instance() : SimServer(static_cast<Sched &>(*this), static_cast<World &>(*this)) {}
