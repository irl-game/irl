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

  void tick();

  auto operator()(const Version &msg) const -> void;
  auto operator()(StartMove msg) -> void;
  auto operator()(StopMove msg) -> void;
  auto operator()(MouseMove msg) -> void;
  auto operator()(const World &) const -> void {}

private:
  std::vector<char> sendBuff;
  Net::Conn *conn{};
  World world;
  enum class MoveFlag: unsigned { Up = 1 << 0, Right = 1 << 1, Down = 1 << 2, Left = 1 << 3 };
  unsigned moveState{};
};
