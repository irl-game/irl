#include "public_key.hpp"
#include <log/log.hpp>
#include <net/conn.hpp>
#include <proto/proto.hpp>
#include <sched/sched.hpp>
#include <sdlpp/sdlpp.hpp>
#include <ser/overloaded.hpp>
#include <sstream>

int main()
{
  sdl::Init init{SDL_INIT_EVERYTHING};
  const auto Width{1280};
  const auto Height{720};
  sdl::Window wind("irl", 63, 126, Width, Height, 0);
  sdl::Renderer rend(wind.get(), -1, 0);
  sdl::EventHandler ev;
  auto done{false};
  ev.quit = [&done](const SDL_QuitEvent &) { done = true; };
  Sched sched;
  Net::Conn conn(sched, PublicKey, "127.0.0.1", 1025);
  auto send = [&conn](const auto &msg) -> void {
    SimProto proto;
    std::vector<char> buff;
    OStrm strm{buff};
    proto.ser(strm, msg);
    conn.send(buff.data(), buff.size());
  };

  conn.onConn = [&]() { LOG("Connected"); };
  conn.onDisconn = [&done]() { done = true; };
  conn.onRecv = [&done, &send](const char *data, size_t sz) {
    SimProto proto;
    IStrm strm(data, data + sz);
    proto.deser(strm,
                overloaded{
                  [](const Message &value) { LOG(typeid(value).name(), value.msg); },
                  [&done, &send](const Version &value) {
                    LOG(typeid(value).name(), value.value);
                    if (value.value != SimProto::version())
                    {
                      LOG("Version mismatch:", value.value, "!=", SimProto::version());
                      done = true;
                    }
                    Message msg;
                    msg.msg = "Hello from client to server!";
                    send(msg);
                  },
                });
  };

  sched.regIdle([&]() {
    while (ev.poll())
      ;
    rend.present();
  });

  while (!done)
    sched.process();
}
