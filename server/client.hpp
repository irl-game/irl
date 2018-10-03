#pragma once
#include <proto/proto.hpp>
#include <net/conn.hpp>

class Client
{
public:
  Client(Net::Conn &conn) noexcept;

  template <typename Msg>
  constexpr auto send(const Msg &msg) -> void
  {
    SimProto proto;
    sendBuff.clear();
    OStrm strm{sendBuff};
    proto.ser(strm, msg);
    conn->send(sendBuff.data(), sendBuff.size());
  }

  auto operator()(const Message &msg) const -> void;
  auto operator()(const Version &msg) const -> void;

private:
  std::vector<char> sendBuff;
  Net::Conn *conn{};
};
