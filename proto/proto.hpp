#pragma once
#include <ser/proto.hpp>

struct Version
{
  uint32_t value{};
  template <typename Arch>
  void ser(Arch &arch)
  {
    arch &value;
  }
};

struct Message
{
  std::string msg;
  template <typename Arch>
  void ser(Arch &arch)
  {
    arch &msg;
  }
};

using SimProto = Proto<Version, Message>;
