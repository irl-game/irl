#include "world.hpp"
#include "client.hpp"
#include "entity.hpp"

World::World() = default;
World::~World() = default;

auto World::tick() noexcept -> void
{
  for (auto &&client : clients)
    client.second->tick();
}
