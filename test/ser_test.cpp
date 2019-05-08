#include <catch2/catch.hpp>
#include <ser/macro.hpp>
#include <ser/ser.hpp>
#include <sstream>
#include <vector>

TEST_CASE("Primitive type", "[ser]")
{
  std::vector<char> buff;
  OStrm ost{buff};
  int32_t a{10};
  ser(ost, a);

  IStrm ist(buff.data(), buff.data() + buff.size());
  int32_t b;
  deser(ist, b);

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

TEST_CASE("Simple class", "[ser]")
{
  std::vector<char> buff;
  OStrm ost{buff};
  A1 a(10);
  ser(ost, a);

  IStrm ist(buff.data(), buff.data() + buff.size());
  A1 b;
  deser(ist, b);

  REQUIRE(b.a == a.a);
}

class A2
{
public:
  std::vector<int> a;
#define SER_PROPERTY_LIST SER_PROPERTY(a);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERY_LIST
};

TEST_CASE("Array", "[ser]")
{
  std::vector<char> buff;
  OStrm ost{buff};
  A2 a;
  a.a = {1, 2, 3};
  ser(ost, a);

  IStrm ist(buff.data(), buff.data() + buff.size());
  A2 b;
  deser(ist, b);

  REQUIRE(b.a == a.a);
}

class A3
{
public:
  std::string a;
#define SER_PROPERTY_LIST SER_PROPERTY(a);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERY_LIST
};

TEST_CASE("String", "[ser]")
{
  std::vector<char> buff;
  OStrm ost{buff};
  A3 a;
  a.a = "Hello world";
  ser(ost, a);

  IStrm ist(buff.data(), buff.data() + buff.size());
  A3 b;
  deser(ist, b);

  REQUIRE(b.a == a.a);
}

class A4
{
public:
  std::unique_ptr<std::string> a;
#define SER_PROPERTY_LIST SER_PROPERTY(a);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};

TEST_CASE("Unique ptr", "[ser]")
{
  std::vector<char> buff;
  OStrm ost{buff};
  A4 a;
  a.a = std::make_unique<std::string>("Hello world");
  ser(ost, a);

  IStrm ist(buff.data(), buff.data() + buff.size());
  A4 b;
  deser(ist, b);

  REQUIRE(*b.a == *a.a);
}

class A5
{
public:
  A4 a;
  std::string b;
#define SER_PROPERTY_LIST \
  SER_PROPERTY(a);        \
  SER_PROPERTY(b);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};

TEST_CASE("Nested class", "[ser]")
{
  std::vector<char> buff;
  OStrm ost{buff};
  A5 a;
  a.a.a = std::make_unique<std::string>("Hello world");
  a.b = "World2";
  ser(ost, a);

  IStrm ist(buff.data(), buff.data() + buff.size());
  A5 b;
  deser(ist, b);

  REQUIRE(*b.a.a == *a.a.a);
  REQUIRE(b.b == a.b);
}

class A6
{
public:
  std::vector<A4> a;
  std::string b;
#define SER_PROPERTY_LIST \
  SER_PROPERTY(a);        \
  SER_PROPERTY(b);
  SER_DEFINE_PROPERTIES()
#undef SER_PROPERTY_LIST
};

TEST_CASE("Nested vector", "[ser]")
{
  std::vector<char> buff;
  OStrm ost{buff};
  A6 a;
  a.a.emplace_back();
  a.a[0].a = std::make_unique<std::string>("test nested vector");
  a.b = "World2";
  ser(ost, a);

  IStrm ist(buff.data(), buff.data() + buff.size());
  A6 b;
  deser(ist, b);

  REQUIRE(b.a.size() == 1);
  REQUIRE(*b.a[0].a == *a.a[0].a);
  REQUIRE(b.b == a.b);
}

enum class E1 { a, b, c, d };

TEST_CASE("Enum", "[ser]")
{
  std::vector<char> buff;
  OStrm ost{buff};
  E1 a{E1::d};
  ser(ost, a);

  IStrm ist(buff.data(), buff.data() + buff.size());
  E1 b;
  deser(ist, b);

  REQUIRE(a == b);
}
