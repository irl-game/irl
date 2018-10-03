#include "sim_server.hpp"
#include "client.hpp"
#include "private_key.hpp"
#include <log/log.hpp>
#include <net/conn.hpp>
#include <proto/proto.hpp>
#include <ser/overloaded.hpp>

SimServer::SimServer(Sched &sched)
  : server(sched, PrivateKey, 1025, [this](Net::Conn *conn) {
      auto client = std::make_unique<Client>(*conn);
      auto clientPtr = client.get();
      clients[clientPtr] = std::move(client);
      conn->onDisconn = [clientPtr, this]() { clients.erase(clientPtr); };
    })
{
}

SimServer::~SimServer() = default;
