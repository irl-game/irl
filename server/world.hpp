#pragma once
#include <memory>
#include <unordered_map>
#include <unordered_set>

class Entity;
class Client;
class World
{
public:
  World();
  ~World();
  auto tick() noexcept -> void;

  std::unordered_map<Client *, std::unique_ptr<Client>> clients;

private:
  std::unordered_map<const Entity *, std::unique_ptr<Entity>> entities;
  std::unordered_map<uint32_t, std::unordered_set<Entity *>> grid;
};