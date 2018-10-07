#pragma once
#include <memory>
#include <net/server.hpp>
#include <sched/sched.hpp>

class Client;
class World;
namespace Net
{
  class Conn;
}
class SimServer
{
public:
  SimServer(Sched &, World &);
  ~SimServer();

private:
  Net::Server server;
  World &world;
  std::unordered_map<Client *, std::unique_ptr<Client>> clients;
  Sched::TimerCanceler timerCanceler;

  auto newConn(Net::Conn *) -> void;
};
