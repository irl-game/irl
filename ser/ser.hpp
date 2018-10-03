#pragma once
#include "istrm.hpp"
#include "ostrm.hpp"
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

template <typename T>
constexpr auto ser(OStrm &strm, const T &value) -> void;

template <typename T>
constexpr auto deser(OStrm &strm, T &value) -> void;

class Ser;
class Deser;

namespace internal
{
  template <typename T>
  struct HasSerMethod
  {
    template <typename, typename>
    class Checker;

    template <typename C>
    static std::true_type test(Checker<C, decltype(&C::ser)> *);

    template <typename C>
    static std::false_type test(...);

    using Type = decltype(test<T>(nullptr));
    static const bool value = std::is_same_v<std::true_type, Type>;
  };

  template <typename T>
  inline constexpr bool HasSerMethodV = HasSerMethod<T>::value;

  template <typename T>
  struct HasDeserMethod
  {
    template <typename, typename>
    class Checker;

    template <typename C>
    static std::true_type test(Checker<C, decltype(&C::deser)> *);

    template <typename C>
    static std::false_type test(...);

    using Type = decltype(test<T>(nullptr));
    static const bool value = std::is_same_v<std::true_type, Type>;
  };

  template <typename T>
  inline constexpr bool HasDeserMethodV = HasDeserMethod<T>::value;
} // namespace internal

class Ser
{
public:
  constexpr Ser(OStrm &strm) : strm(strm) {}

  template <typename T>
  constexpr auto operator<<(const T &value) -> Ser &
  {
    if constexpr (internal::HasSerMethodV<T>)
      value.ser(*this);
    else
      serVal(value);
    return *this;
  }

  template <typename T>
  constexpr auto serVal(const T value) noexcept
    -> std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>
  {
    strm.write((char *)&value, sizeof(value));
  }

  auto serVal(const std::string &value) noexcept -> void;

  template <typename T>
  constexpr auto serVal(const std::vector<T> &value) -> void
  {
    int32_t sz = value.size();
    *this << sz;
    for (auto &&v : value)
      *this << v;
  }

  template <typename T>
  constexpr auto serVal(const std::unique_ptr<T> &value) -> void
  {
    int32_t isNull = value ? 0 : 1;
    *this << isNull;
    if (value)
      *this << *value;
  }

private:
  OStrm &strm;
};

class Deser
{
public:
  constexpr Deser(IStrm &strm) : strm(strm) {}

  template <typename T>
  constexpr auto deserVal(T &value) noexcept
    -> std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>
  {
    strm.read((char *)&value, sizeof(value));
  }

  auto deserVal(std::string &value) noexcept -> void;

  template <typename T>
  constexpr auto operator>>(T &value) -> Deser &
  {
    if constexpr (internal::HasDeserMethodV<T>)
      value.deser(*this);
    else
      deserVal(value);
    return *this;
  }

  template <typename T>
  constexpr auto deserVal(std::vector<T> &value) -> void
  {
    value.clear();
    int32_t sz{};
    *this >> sz;
    value.reserve(sz);
    for (auto i = 0; i < sz; ++i)
    {
      T &v = value.emplace_back();
      *this >> v;
    }
  }

  template <typename T>
  constexpr auto deserVal(std::unique_ptr<T> &value) -> void
  {
    int32_t isNull{};
    *this >> isNull;
    if (isNull == 1)
    {
      value = nullptr;
      return;
    }

    value = std::make_unique<T>();
    *this >> *value;
  }

private:
  IStrm &strm;
};

template <typename T>
constexpr auto ser(OStrm &strm, const T &value) -> void
{
  Ser s(strm);
  if constexpr (internal::HasSerMethodV<T>)
    value.ser(s);
  else
    s << value;
}

template <typename T>
constexpr auto deser(IStrm &strm, T &value) -> void
{
  Deser s(strm);
  if constexpr (internal::HasDeserMethodV<T>)
    value.deser(s);
  else
    s >> value;
}
