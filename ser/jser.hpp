#pragma once
#include "is_serializable.hpp"
#include <json/json.h>

template <typename T>
void jser(Json::Value &jsonValue, const T &value);

template <typename T>
void jdeser(const Json::Value &jsonValue, T &value);

namespace internal
{
  class JSer
  {
  public:
    constexpr JSer(Json::Value &jsonValue) : jsonValue(jsonValue) {}
    template <typename T>
    constexpr auto operator()(const char *name, const T &value) -> void
    {
      if constexpr (internal::IsSerializableClassV<T>)
        value.ser(JSer{jsonValue[name]});
      else
        jser(jsonValue[name], value);
    }

  private:
    Json::Value &jsonValue;
  };

  class JDeser
  {
  public:
    constexpr JDeser(const Json::Value &jsonValue) : jsonValue(jsonValue) {}
    template <typename T>
    constexpr auto operator()(const char *name, T &value) const -> void
    {
      if constexpr (internal::IsSerializableClassV<T>)
        value.deser(JDeser{jsonValue[name]});
      else
        jdeser(jsonValue[name], value);
    }

  private:
    const Json::Value &jsonValue;
  };
} // namespace internal

template <typename T>
void jser(Json::Value &jsonValue, const T &value)
{
  internal::JSer s(jsonValue);
  if constexpr (internal::IsSerializableClassV<T>)
    value.ser(s);
  else if constexpr (std::is_enum_v<T>)
    jsonValue = static_cast<int>(value);
  else
    jsonValue = value;
}

template <typename T>
void jdeser(const Json::Value &jsonValue, T &value)
{
  internal::JDeser s(jsonValue);
  if constexpr (internal::IsSerializableClassV<T>)
    value.deser(s);
  else if constexpr (std::is_integral_v<T>)
    value = jsonValue.asInt64();
  else if constexpr (std::is_enum_v<T>)
    value = static_cast<T>(jsonValue.asInt64());
  else if constexpr (std::is_floating_point_v<T>)
    value = jsonValue.asDouble();
}

// TODO std::vector, std::optional, std::string