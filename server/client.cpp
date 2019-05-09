#include "client.hpp"
#include "world.hpp"
#include <cmath>
#include <log/log.hpp>
#include <pi/pi.hpp>

Client::Client(Net::Conn &conn, World &world) noexcept : conn(conn), world(world)
{
  LOG("New client: ", this, &conn);
  conn.onRecv = [this](const char *data, size_t sz) {
    SimProto proto;
    IStrm strm{data, data + sz};
    proto.deser(strm, *this);
  };
  proto::Version ver{SimProto::version()};
  send(ver);
}

auto Client::operator()(proto::Version msg) const -> void
{
  LOG(typeid(msg).name(), msg.value);
}

auto Client::operator()(proto::KeysState msg) -> void
{
  LOG("Keys State", static_cast<unsigned>(msg));
  keysState = msg;
}

auto Client::operator()(proto::MouseMove msg) -> void
{
  LOG("Mouse move", msg.x, msg.y);
  auto &dir = heroView.hero.dir;
  dir.ang1 -= 0.003f * msg.x;
  while (dir.ang1 < 0)
    dir.ang1 += 2.0f * Pi;
  while (dir.ang1 > 2.0f * Pi)
    dir.ang1 -= 2.0f * Pi;
  dir.ang2 = std::clamp(dir.ang2 + 0.003f * msg.y, -Pi / 2.0f, Pi / 2.0f);
}

void Client::tick()
{
  auto &hero = heroView.hero;
  heroView.stones.clear();
  for (const auto &c : world.clients)
    heroView.stones.push_back(
      proto::Stone{c.first->heroView.hero.pos.x, c.first->heroView.hero.pos.y});
  for (int x = 0; x < 4; ++x)
    for (int y = 0; y < 4; ++y)
      heroView.stones.push_back(
        proto::Stone{static_cast<float>(3.0f * x), static_cast<float>(3.0f * y)});
  send(heroView);

  const auto MoveSpeed = 0.4f;

  if (keysState & proto::KeysState::Up)
  {
    const auto dx = MoveSpeed * cos(hero.dir.ang1);
    const auto dy = MoveSpeed * sin(hero.dir.ang1);
    hero.pos.x += dx;
    hero.pos.y += dy;
  }
  if (keysState & proto::KeysState::Right)
  {
    const auto dx = MoveSpeed * cos(hero.dir.ang1 - Pi / 2.0f);
    const auto dy = MoveSpeed * sin(hero.dir.ang1 - Pi / 2.0f);
    hero.pos.x += dx;
    hero.pos.y += dy;
  }
  if (keysState & proto::KeysState::Down)
  {
    const auto dx = MoveSpeed * cos(hero.dir.ang1);
    const auto dy = MoveSpeed * sin(hero.dir.ang1);
    hero.pos.x -= dx;
    hero.pos.y -= dy;
  }
  if (keysState & proto::KeysState::Left)
  {
    const auto dx = MoveSpeed * cos(hero.dir.ang1 + Pi / 2.0f);
    const auto dy = MoveSpeed * sin(hero.dir.ang1 + Pi / 2.0f);
    hero.pos.x += dx;
    hero.pos.y += dy;
  }
}
