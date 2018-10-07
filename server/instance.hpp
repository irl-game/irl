#pragma once
#include "sim_server.hpp"
#include "world.hpp"
#include <sched/sched.hpp>

class Instance : public Sched, public World, public SimServer
{
public:
  Instance();
};
