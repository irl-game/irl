#include "client.hpp"
#include <log/log.hpp>

Client::Client(Net::Conn &conn) noexcept : conn(&conn)
{
  LOG("New client: ", this, &conn);
  conn.onRecv = [this](const char *data, size_t sz) {
    SimProto proto;
    IStrm strm{data, data + sz};
    proto.deser(strm, *this);
  };
  Version ver{SimProto::version()};
  send(ver);
}

auto Client::operator()(const Message &msg) const -> void
{
  LOG(typeid(msg).name(), msg.msg);
}

auto Client::operator()(const Version &msg) const -> void
{
  LOG(typeid(msg).name(), msg.value);
}
