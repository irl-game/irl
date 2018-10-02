#pragma once
#include <ser/proto.hpp>

struct Version
{
  uint32_t value{};
  void ser(Ser &arch) const { arch << value; }
  void deser(Deser &arch) { arch >> value; }
};

struct Message
{
  std::string msg;
  void ser(Ser &arch) const { arch << msg; }
  void deser(Deser &arch) { arch >> msg; }
};

using SimProto = Proto<Version, Message>;
