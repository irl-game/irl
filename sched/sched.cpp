#include "sched.hpp"

Sched::Sched()
{
  uv_loop_init(&loop);
  uv_idle_init(&loop, &idle);
  idle.data = this;
}

Sched::~Sched()
{
  if (idleFunc)
    uv_idle_stop(&idle);
  uv_loop_close(&loop);
}

void Sched::process()
{
  uv_run(&loop, UV_RUN_ONCE);
}

void Sched::regIdle(std::function<void()> &&func)
{
  if (!func)
  {
    uv_idle_stop(&idle);
    return;
  }

  idleFunc = func;
  uv_idle_start(&idle, [](uv_idle_t *ctx) { static_cast<Sched *>(ctx->data)->idleFunc(); });
}
