#include "client.hpp"
#include <cmath>
#include <log/log.hpp>

Client::Client(Net::Conn &conn) noexcept : conn(&conn)
{
  LOG("New client: ", this, &conn);
  for (auto i = 0; i < 100; ++i)
    world.stones.push_back(Stone{1.0f * (rand() % 100), 1.0f * (rand() % 100)});
  conn.onRecv = [this](const char *data, size_t sz) {
    SimProto proto;
    IStrm strm{data, data + sz};
    proto.deser(strm, *this);
  };
  Version ver{SimProto::version()};
  send(ver);
}

auto Client::operator()(const Version &msg) const -> void
{
  LOG(typeid(msg).name(), msg.value);
}

auto Client::operator()(StartMove msg) -> void
{
  LOG("Start move", static_cast<int>(msg));
  switch (msg)
  {
  case StartMove::Up: moveState |= static_cast<unsigned>(MoveFlag::Up); break;
  case StartMove::Right: moveState |= static_cast<unsigned>(MoveFlag::Right); break;
  case StartMove::Down: moveState |= static_cast<unsigned>(MoveFlag::Down); break;
  case StartMove::Left: moveState |= static_cast<unsigned>(MoveFlag::Left); break;
  }
}

auto Client::operator()(StopMove msg) -> void
{
  LOG("Stop move", static_cast<int>(msg));
  switch (msg)
  {
  case StopMove::Up: moveState &= ~static_cast<unsigned>(MoveFlag::Up); break;
  case StopMove::Right: moveState &= ~static_cast<unsigned>(MoveFlag::Right); break;
  case StopMove::Down: moveState &= ~static_cast<unsigned>(MoveFlag::Down); break;
  case StopMove::Left: moveState &= ~static_cast<unsigned>(MoveFlag::Left); break;
  }
}

auto Client::operator()(MouseMove msg) -> void
{
  world.hero.dir.ang1 -= 0.003f * msg.x;
  world.hero.dir.ang2 += 0.003f * msg.y;
}

void Client::tick()
{
  send(world);

  auto MoveSpeed = 0.4f;

  if ((moveState & static_cast<unsigned>(MoveFlag::Up)) != 0)
  {
    const auto dx = MoveSpeed * cos(world.hero.dir.ang1);
    const auto dy = MoveSpeed * sin(world.hero.dir.ang1);
    world.hero.pos.x += dx;
    world.hero.pos.y += dy;
  }
  if ((moveState & static_cast<unsigned>(MoveFlag::Right)) != 0)
  {
    const auto dx = MoveSpeed * cos(world.hero.dir.ang1 - 3.1415926f / 2.0f);
    const auto dy = MoveSpeed * sin(world.hero.dir.ang1 - 3.1415926f / 2.0f);
    world.hero.pos.x += dx;
    world.hero.pos.y += dy;
  }
  if ((moveState & static_cast<unsigned>(MoveFlag::Down)) != 0)
  {
    const auto dx = MoveSpeed * cos(world.hero.dir.ang1);
    const auto dy = MoveSpeed * sin(world.hero.dir.ang1);
    world.hero.pos.x -= dx;
    world.hero.pos.y -= dy;
  }
  if ((moveState & static_cast<unsigned>(MoveFlag::Left)) != 0)
  {
    const auto dx = MoveSpeed * cos(world.hero.dir.ang1 + 3.1415926f / 2.0f);
    const auto dy = MoveSpeed * sin(world.hero.dir.ang1 + 3.1415926f / 2.0f);
    world.hero.pos.x += dx;
    world.hero.pos.y += dy;
  }
}
