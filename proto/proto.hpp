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

struct MouseMove
{
  int32_t x{};
  int32_t y{};
  void ser(Ser &arch) const { arch << x << y; }
  void deser(Deser &arch) { arch >> x >> y; }
};

struct Pos
{
  float x{};
  float y{};
  float z{};
  void ser(Ser &arch) const { arch << x << y << z; }
  void deser(Deser &arch) { arch >> x >> y >> z; }
};

struct Direction
{
  float ang1{};
  float ang2{};
  void ser(Ser &arch) const { arch << ang1 << ang2; }
  void deser(Deser &arch) { arch >> ang1 >> ang2; }
};

struct Hero
{
  Pos pos;
  Direction dir;
  void ser(Ser &arch) const { arch << pos << dir; }
  void deser(Deser &arch) { arch >> pos >> dir; }
};

struct Stone
{
  Pos pos;
  void ser(Ser &arch) const { arch << pos; }
  void deser(Deser &arch) { arch >> pos; }
};

struct World
{
  Hero hero;
  std::vector<Stone> stones;
  void ser(Ser &arch) const { arch << hero << stones; }
  void deser(Deser &arch) { arch >> hero >> stones; }
};

using SimProto = Proto<Version, StartMove, StopMove, MouseMove, World>;
