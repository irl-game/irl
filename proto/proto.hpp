#pragma once
#include <ser/proto.hpp>

struct Version
{
  uint32_t value{};
  void ser(Ser &arch) const { arch << value; }
  void deser(Deser &arch) { arch >> value; }
};

enum class StartMove : int32_t { Up, Right, Down, Left };
enum class StopMove : int32_t { Up, Right, Down, Left };

struct Pos
{
  int32_t x{};
  int32_t y{};
  void ser(Ser &arch) const { arch << x << y; }
  void deser(Deser &arch) { arch >> x >> y; }
};

using SimProto = Proto<Version, StartMove, StopMove, Pos>;
