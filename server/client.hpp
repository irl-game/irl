#pragma once
#include <proto/proto.hpp>
#include <net/conn.hpp>

class World;
class Client
{
public:
  Client(Net::Conn &conn, World &) noexcept;

  template <typename Msg>
  constexpr auto send(const Msg &msg) -> void
  {
    SimProto proto;
    sendBuff.clear();
    OStrm strm{sendBuff};
    proto.ser(strm, msg);
    conn.send(sendBuff.data(), sendBuff.size());
  }

  void tick();

  auto operator()(proto::Version) const -> void;
  auto operator()(const proto::HeroView &) const -> void {}
  auto operator()(proto::KeysState) -> void;
  auto operator()(proto::MouseMove) -> void;

private:
  std::vector<char> sendBuff;
  Net::Conn &conn;
  World &world;
  proto::HeroView heroView;
  proto::KeysState keysState{};
};
