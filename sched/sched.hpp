#pragma once
#include <functional>
#include <uv.h>

class Sched
{
public:
  Sched();
  ~Sched();
  void process();
  void regIdle(std::function<void()>&&);
  uv_loop_t loop;

private:
  uv_idle_t idle;
  std::function<void()> idleFunc;
};
