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
  proto::KeysState keysState{};
  ev.keyDown = [&conn, &keysState](const SDL_KeyboardEvent &key) {
    switch (key.keysym.sym)
    {
    case SDLK_w: keysState |= proto::KeysState::Up; break;
    case SDLK_s: keysState |= proto::KeysState::Down; break;
    case SDLK_a: keysState |= proto::KeysState::Left; break;
    case SDLK_d: keysState |= proto::KeysState::Right; break;
    }
    conn.send(keysState);
  };
  ev.keyUp = [&conn, &keysState](const SDL_KeyboardEvent &key) {
    switch (key.keysym.sym)
    {
    case SDLK_w: keysState &= ~proto::KeysState::Up; break;
    case SDLK_s: keysState &= ~proto::KeysState::Down; break;
    case SDLK_a: keysState &= ~proto::KeysState::Left; break;
    case SDLK_d: keysState &= ~proto::KeysState::Right; break;
    }
    conn.send(keysState);
  };
  ev.mouseMotion = [&conn](const SDL_MouseMotionEvent &mouse) {
    conn.send(proto::MouseMove{mouse.xrel, mouse.yrel});
  };

  sched.regIdle([&]() {
    while (ev.poll())
      ;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shad.use();

    const auto &hero = conn.heroView.hero;
    const auto d = glm::rotate(hero.dir.ang1, glm::vec3{0.0f, 0.0f, 1.0f}) *
                   glm::rotate(hero.dir.ang2, glm::vec3{0.0f, 1.0f, 0.0f}) *
                   glm::vec4{100.0f, 0.0f, 0.0f, 1.0f};

    view = glm::lookAt(
      glm::vec3(hero.pos.x, hero.pos.y, hero.pos.z + 3.0f),
      glm::vec3(hero.pos.x + d.x,
                hero.pos.y + d.y,
                hero.pos.z + 3.0f + d.z), // and looks at the origin
      glm::vec3(0, 0, 1));
    view.update();

    for (auto &&st : conn.heroView.stones)
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
