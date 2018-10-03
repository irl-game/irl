#include "sim_conn.hpp"
#include <sched/sched.hpp>
#include <sdlpp/sdlpp.hpp>
#include <shade/library.hpp>
#include <shade/obj.hpp>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
int main()
{
  sdl::Init init{SDL_INIT_EVERYTHING};
  const auto Width{1280};
  const auto Height{720};
  sdl::Window win("irl", 63, 126, Width, Height, SDL_WINDOW_OPENGL);
  sdl::Renderer rend(win.get(), -1, 0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  sdl::EventHandler ev;
  auto done{false};
  ev.quit = [&done](const SDL_QuitEvent &) { done = true; };
  Library lib(rend.get());
  auto bot = lib.getObj("bot");

  Sched sched;
  SimConn conn(sched, "127.0.0.1", 1025);

  sched.regIdle([&]() {
    while (ev.poll())
      ;

    rend.clear();
    bot->draw();
    rend.present();
  });

  while (!done)
    sched.process();
}
