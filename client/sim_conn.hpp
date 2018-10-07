#pragma once
#include <memory>
#include <net/conn.hpp>
#include <proto/proto.hpp>

namespace Net
{
  class Conn;
};

class Sched;

class SimConn
{
public:
  SimConn(Sched &, const std::string &addr, int port);
  ~SimConn();

  proto::HeroView heroView;

  template <typename T>
  auto send(const T &msg) -> void
  {
    buff.clear();
    SimProto proto;
    OStrm strm{buff};
    proto.ser(strm, msg);
    conn->send(buff.data(), buff.size());
  };
  auto operator()(proto::Version value) -> void;
  auto operator()(proto::KeysState) -> void {}
  auto operator()(proto::MouseMove) -> void {}
  auto operator()(proto::HeroView &&value) -> void;

private:
  std::vector<char> buff;
  std::unique_ptr<Net::Conn> conn;
};
