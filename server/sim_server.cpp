#include "sim_server.hpp"
#include "client.hpp"
#include "private_key.hpp"
#include "world.hpp"
#include <chrono>
#include <log/log.hpp>
#include <net/conn.hpp>
#include <proto/proto.hpp>
#include <sched/sched.hpp>
#include <ser/overloaded.hpp>

using namespace std::chrono_literals;
SimServer::SimServer(Sched &sched, World &world)
  : server(sched, PrivateKey, 1025, [this](Net::Conn *conn) { newConn(conn); }),
    world(world),
    timerCanceler(sched.regTimer([this]() { this->world.tick(); }, 10ms, true))
{
}

SimServer::~SimServer()
{
  timerCanceler();
}

auto SimServer::newConn(Net::Conn *conn) -> void
{
  auto client = std::make_unique<Client>(*conn, world);
  auto clientPtr = client.get();
  world.clients[clientPtr] = std::move(client);
  conn->onDisconn = [clientPtr, this]() { world.clients.erase(clientPtr); };
}
