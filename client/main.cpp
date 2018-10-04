#include "sim_conn.hpp"
#include <sched/sched.hpp>
#include <sdlpp/sdlpp.hpp>
#include <shade/library.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>
#include <shade/var.hpp>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

int main()
{
  sdl::Init init{SDL_INIT_EVERYTHING};
  const auto ScreenWidth{1280};
  const auto ScreenHeight{720};
  sdl::Window win("irl", 63, 100, ScreenWidth, ScreenHeight, SDL_WINDOW_OPENGL);
  sdl::Renderer rend(win.get(), -1, 0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  sdl::EventHandler ev;
  auto done{false};
  ev.quit = [&done](const SDL_QuitEvent &) { done = true; };
  Library lib(rend.get());
  auto bot = lib.getObj("bot");
  Var<glm::mat4> proj{"proj"};
  Var<glm::mat4> view{"view"};
  Var<glm::mat4> mvp{"mvp"};
  proj = glm::perspective(glm::radians(45.0f), 1.0f * ScreenWidth / ScreenHeight, 0.1f, 1000.0f);
  auto camZ = 100.0f;
  view = glm::lookAt(glm::vec3(0.0f, 0.0f - camZ, camZ),
                     glm::vec3(0.0f, 0.0f, 0.0f), // and looks at the origin
                     glm::vec3(0, 0, 1));
  mvp = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

  ShaderProgram shad{"shad", "shad", mvp, proj, view};

  Sched sched;
  SimConn conn(sched, "127.0.0.1", 1025);
  ev.keyDown = [&conn](const SDL_KeyboardEvent &key) {
    switch (key.keysym.sym)
    {
    case SDLK_UP: conn.send(StartMove::Up); break;
    case SDLK_RIGHT: conn.send(StartMove::Right); break;
    case SDLK_DOWN: conn.send(StartMove::Down); break;
    case SDLK_LEFT: conn.send(StartMove::Left); break;
    }
  };
  ev.keyUp = [&conn](const SDL_KeyboardEvent &key) {
    switch (key.keysym.sym)
    {
    case SDLK_UP: conn.send(StopMove::Up); break;
    case SDLK_RIGHT: conn.send(StopMove::Right); break;
    case SDLK_DOWN: conn.send(StopMove::Down); break;
    case SDLK_LEFT: conn.send(StopMove::Left); break;
    }
  };

  sched.regIdle([&]() {
    while (ev.poll())
      ;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mvp = glm::translate(glm::vec3(1.0f * conn.pos.x, 1.0f * conn.pos.y, 0.0f));
    shad.use();
    bot->draw();
    rend.present();
  });

  while (!done)
    sched.process();
}
