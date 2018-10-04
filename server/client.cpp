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

auto Client::operator()(const Version &msg) const -> void
{
  LOG(typeid(msg).name(), msg.value);
}

auto Client::operator()(StartMove msg) -> void
{
  LOG("Start move", static_cast<int>(msg));
  switch (msg)
  {
  case StartMove::Up: ++pos.y; break;
  case StartMove::Right: ++pos.x; break;
  case StartMove::Down: --pos.y; break;
  case StartMove::Left: --pos.x; break;
  }
}

auto Client::operator()(StopMove msg) -> void
{
  LOG("Stop move", static_cast<int>(msg));
}

auto Client::operator()(Pos) const -> void {}

void Client::tick()
{
  send(pos);
}
