#pragma once
#include <memory>
#include <unordered_map>
#include <unordered_set>

class Entity;
class World
{
public:
  World();
  ~World();

private:
  std::unordered_map<const Entity *, std::unique_ptr<Entity>> entities;
  std::unordered_map<uint32_t, std::unordered_set<Entity *>> grid;
};