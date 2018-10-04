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

  Pos pos;

  template <typename T>
  auto send(const T &msg) -> void
  {
    buff.clear();
    SimProto proto;
    OStrm strm{buff};
    proto.ser(strm, msg);
    conn->send(buff.data(), buff.size());
  };
  auto operator()(const Version &value) -> void;
  auto operator()(StartMove) -> void {}
  auto operator()(StopMove) -> void {}
  auto operator()(Pos value) -> void;

private:
  std::vector<char> buff;
  std::unique_ptr<Net::Conn> conn;
};
