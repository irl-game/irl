#pragma once
#include <ser/proto.hpp>
#include <ser/macro.hpp>

namespace proto
{
  struct Version
  {
    uint32_t value{};
#define SER_PROPERTY_LIST SER_PROPERTY(value);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };

  enum class KeysState : uint32_t { Up = 1 << 0, Right = 1 << 1, Down = 1 << 2, Left = 1 << 3 };

  constexpr auto operator&(KeysState value, KeysState flag) -> bool
  {
    return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
  }

  constexpr auto operator|=(KeysState &value, KeysState flag) -> KeysState &
  {
    value = static_cast<KeysState>(static_cast<uint32_t>(value) | static_cast<uint32_t>(flag));
    return value;
  }

  constexpr auto operator&=(KeysState &value, KeysState flag) -> KeysState &
  {
    value = static_cast<KeysState>(static_cast<uint32_t>(value) & static_cast<uint32_t>(flag));
    return value;
  }
  constexpr auto operator~(KeysState value) -> KeysState
  {
    return static_cast<KeysState>(~static_cast<uint32_t>(value));
  }

  struct MouseMove
  {
    int32_t x{};
    int32_t y{};
#define SER_PROPERTY_LIST\
    SER_PROPERTY(x); \
    SER_PROPERTY(y);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };

  struct Pos
  {
    float x{};
    float y{};
    float z{};
#define SER_PROPERTY_LIST\
    SER_PROPERTY(x); \
    SER_PROPERTY(y); \
    SER_PROPERTY(z);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };

  struct Direction
  {
    float ang1{};
    float ang2{};
#define SER_PROPERTY_LIST\
    SER_PROPERTY(ang1); \
    SER_PROPERTY(ang2);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };

  struct Hero
  {
    Pos pos;
    Direction dir;
#define SER_PROPERTY_LIST\
    SER_PROPERTY(pos); \
    SER_PROPERTY(dir);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };

  struct Stone
  {
    Pos pos;
#define SER_PROPERTY_LIST\
    SER_PROPERTY(pos);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };

  struct HeroView
  {
    Hero hero;
    std::vector<Stone> stones;
#define SER_PROPERTY_LIST\
    SER_PROPERTY(hero); \
    SER_PROPERTY(stones);
    SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
  };
} // namespace proto

using SimProto = Proto<proto::Version, proto::KeysState, proto::MouseMove, proto::HeroView>;
