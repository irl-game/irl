#include <catch2/catch.hpp>
#include <ser/jser.hpp>
#include <ser/macro.hpp>

TEST_CASE("Serialize int to json", "[jser]")
{
  Json::Value root;
  int32_t a{10};
  jser(root, a);
  REQUIRE(root == 10);

  int32_t b;
  jdeser(root, b);

  REQUIRE(a == b);
}

TEST_CASE("Serialize float to json", "[jser]")
{
  Json::Value root;
  float a{3.14f};
  jser(root, a);
  REQUIRE(root == 3.14f);

  float b;
  jdeser(root, b);

  REQUIRE(a == b);
}

enum class MyEnum { a, b, c };

TEST_CASE("Serialize enum to json", "[jser]")
{
  Json::Value root;
  MyEnum a{MyEnum::c};
  jser(root, a);
  REQUIRE(root == static_cast<int>(MyEnum::c));

  MyEnum b;
  jdeser(root, b);

  REQUIRE(a == b);
}

class A1
{
public:
  A1(int a = 0) : a(a) {}
  int a;
#define SER_PROPERTY_LIST SER_PROPERTY(a);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERY_LIST
};

TEST_CASE("Serialize simple class to json", "[jser]")
{
  Json::Value root;
  A1 a(10);
  jser(root, a);
  REQUIRE(root["a"] == a.a);

  A1 b;
  jdeser(root, b);

  REQUIRE(b.a == a.a);
}
