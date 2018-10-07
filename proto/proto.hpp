#pragma once
#include <ser/proto.hpp>

namespace proto
{
  struct Version
  {
    uint32_t value{};
    void ser(Ser &arch) const { arch << value; }
    void deser(Deser &arch) { arch >> value; }
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

  struct HeroView
  {
    Hero hero;
    std::vector<Stone> stones;
    void ser(Ser &arch) const { arch << hero << stones; }
    void deser(Deser &arch) { arch >> hero >> stones; }
  };
} // namespace proto

using SimProto = Proto<proto::Version, proto::KeysState, proto::MouseMove, proto::HeroView>;
