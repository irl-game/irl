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
  SDL_SetRelativeMouseMode(SDL_TRUE);
  sdl::Renderer rend(win.get(), -1, 0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  sdl::EventHandler ev;
  auto done{false};
  ev.quit = [&done](const SDL_QuitEvent &) { done = true; };
  Library lib(rend.get());
  // auto bot = lib.getObj("bot");
  auto stone = lib.getObj("stone_lvl_0");
  Var<glm::mat4> proj{"proj"};
  Var<glm::mat4> view{"view"};
  Var<glm::mat4> mvp{"mvp"};
  proj = glm::perspective(glm::radians(45.0f), 1.0f * ScreenWidth / ScreenHeight, 0.1f, 1000.0f);
  mvp = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

  ShaderProgram shad{"shad", "shad", mvp, proj, view};

  Sched sched;
  SimConn conn(sched, "127.0.0.1", 1025);
  ev.keyDown = [&conn](const SDL_KeyboardEvent &key) {
    switch (key.keysym.sym)
    {
    case SDLK_w: conn.send(StartMove::Up); break;
    case SDLK_s: conn.send(StartMove::Down); break;
    case SDLK_a: conn.send(StartMove::Left); break;
    case SDLK_d: conn.send(StartMove::Right); break;
    }
  };
  ev.keyUp = [&conn](const SDL_KeyboardEvent &key) {
    switch (key.keysym.sym)
    {
    case SDLK_w: conn.send(StopMove::Up); break;
    case SDLK_s: conn.send(StopMove::Down); break;
    case SDLK_a: conn.send(StopMove::Left); break;
    case SDLK_d: conn.send(StopMove::Right); break;
    }
  };
  ev.mouseMotion = [&conn](const SDL_MouseMotionEvent &mouse) {
    conn.send(MouseMove{mouse.xrel, mouse.yrel});
  };

  sched.regIdle([&]() {
    while (ev.poll())
      ;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shad.use();

    auto dx = 100.f * cos(conn.world.hero.dir.ang1);
    auto dy = 100.f * sin(conn.world.hero.dir.ang1);
    view = glm::lookAt(
      glm::vec3(conn.world.hero.pos.x, conn.world.hero.pos.y, conn.world.hero.pos.z + 3.0f),
      glm::vec3(conn.world.hero.pos.x + dx,
                conn.world.hero.pos.y + dy,
                conn.world.hero.pos.z + 3.0f), // and looks at the origin
      glm::vec3(0, 0, 1));
    view.update();

    for (auto &&st : conn.world.stones)
    {
      mvp = glm::translate(glm::vec3(st.pos.x, st.pos.y, 0.0f));
      mvp.update();
      stone->draw();
    }

    rend.present();
  });

  while (!done)
    sched.process();
}
