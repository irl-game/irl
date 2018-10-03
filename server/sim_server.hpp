#pragma once
#include <net/server.hpp>
#include <memory>

class Sched;
class Client;
class SimServer
{
public:
  SimServer(Sched &);
  ~SimServer();

private:
  Net::Server server;
  std::unordered_map<Client *, std::unique_ptr<Client>> clients;
};
