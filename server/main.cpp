#include "sim_server.hpp"
#include <sched/sched.hpp>

int main()
{
  Sched sched;
  SimServer server(sched);
  for (;;)
    sched.process();
}
