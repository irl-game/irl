#pragma once

namespace proto
{
  class HeroView;
}

class Entity
{
public:
  virtual ~Entity() = default;
  virtual auto addTo(proto::HeroView &) -> void = 0;
  constexpr auto getX() const -> float { return x; }
  constexpr auto getY() const -> float { return y; }

private:
  float x{};
  float y{};
};
