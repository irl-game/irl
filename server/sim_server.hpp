#pragma once
#include <memory>
#include <net/server.hpp>
#include <sched/sched.hpp>

class Client;
class SimServer
{
public:
  SimServer(Sched &);
  ~SimServer();

private:
  Net::Server server;
  std::unordered_map<Client *, std::unique_ptr<Client>> clients;
  Sched::TimerCanceler timerCanceler;
};
