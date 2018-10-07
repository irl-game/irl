#include "sim_conn.hpp"
#include "public_key.hpp"
#include <log/log.hpp>
#include <proto/proto.hpp>
#include <ser/overloaded.hpp>

SimConn::SimConn(Sched &sched, const std::string &addr, int port)
  : conn(std::make_unique<Net::Conn>(sched, PublicKey, addr, port))
{
  conn->onConn = []() { LOG("Connected"); };
  conn->onRecv = [this](const char *data, size_t sz) {
    SimProto proto;
    IStrm strm(data, data + sz);
    proto.deser(strm, *this);
  };
}

SimConn::~SimConn() = default;

auto SimConn::operator()(proto::Version value) -> void
{
  LOG(typeid(value).name(), value.value);
  if (value.value != SimProto::version())
    LOG("Version mismatch:", value.value, "!=", SimProto::version());
}

auto SimConn::operator()(proto::HeroView &&value) -> void
{
  heroView = value;
}
