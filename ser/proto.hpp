#pragma once
#include "istrm.hpp"
#include "ostrm.hpp"
#include "ser.hpp"
#include <array>
#include <sstream>
#include <tomcrypt.h>

namespace internal
{
  template <typename X, typename H, typename... T>
  struct IndexOf
  {
    static const int32_t Value = []() {
      if constexpr (std::is_same_v<H, X>)
        return 0;
      else
        return IndexOf<X, T...>::Value + 1;
    }();
  };

  template <typename H, typename... T>
  struct SerAllDefCtord
  {
  public:
    constexpr auto operator()(OStrm &strm) const -> void
    {
      H h{};
      ::ser(strm, h);
      SerAllDefCtord<T...>{}(strm);
    }
  };

  template <typename H>
  struct SerAllDefCtord<H>
  {
  public:
    constexpr auto operator()(OStrm &strm) const -> void
    {
      H h{};
      ::ser(strm, h);
    }
  };

} // namespace internal

template <typename... Args>
class Proto
{
public:
  template <typename Msg>
  constexpr auto ser(OStrm &strm, const Msg &msg) const -> void
  {
    auto msgId = internal::IndexOf<Msg, Args...>::Value;
    ::ser(strm, msgId);
    ::ser(strm, msg);
  }

  template <typename Vis>
  constexpr auto deser(IStrm &strm, Vis &&vis) const -> void
  {
    int32_t msgId{};
    ::deser(strm, msgId);
    deserById(0, strm, std::move(vis), msgId, Args{}...);
  }

  static auto version() -> uint32_t
  {
    static uint32_t version = calcMd5Hash();
    return version;
  }

private:
  template <typename Arg, typename Vis, typename... Tail>
  constexpr auto deserById(int32_t idx, IStrm &strm, Vis &&vis, int32_t msgId, Arg, Tail... tail)
    const -> void
  {
    if (msgId != idx)
      return deserById(idx + 1, strm, std::move(vis), msgId, tail...);
    Arg m;
    ::deser(strm, m);
    vis(m);
  }

  template <typename Vis>
  constexpr auto deserById(int32_t, IStrm &, Vis &&, int32_t msgId) const -> void
  {
    throw std::runtime_error("Unknown msgId: " + std::to_string(msgId));
  }

  static auto calcMd5Hash() -> uint32_t
  {
    std::vector<char> buff;
    OStrm strm(buff);
    internal::SerAllDefCtord<Args...>{}(strm);
    hash_state md5;
    md5_init(&md5);
    md5_process(&md5, (const unsigned char *)buff.data(), buff.size());
    union
    {
      std::array<unsigned char, 16> hash;
      uint32_t res;
    } hash;
    md5_done(&md5, hash.hash.data());
    return hash.res;
  }
};
