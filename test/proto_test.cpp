#include <catch2/catch.hpp>
#include <ser/overloaded.hpp>
#include <ser/proto.hpp>
#include <ser/ser.hpp>
#include <sstream>

class A
{
public:
  int a = 0;
  void ser(Ser &arch) const { arch << a; }
  void deser(Deser &arch) { arch >> a; }
};

class B
{
public:
  int a = 0;
  void ser(Ser &arch) const { arch << a; }
  void deser(Deser &arch) { arch >> a; }
};

TEST_CASE("Simple A", "[proto]")
{
  A a;
  Proto<A, B> p;
  std::vector<char> buff;
  OStrm ost(buff);
  p.ser(ost, a);
  IStrm ist{buff.data(), buff.data() + buff.size()};
  auto isRecvdA{false};
  auto isRecvdB{false};
  p.deser(ist,
          overloaded{[&isRecvdA](const A &) { isRecvdA = true; },
                     [&isRecvdB](const B &) { isRecvdB = true; }});
  REQUIRE(isRecvdA);
  REQUIRE(!isRecvdB);
}

TEST_CASE("Simple B", "[proto]")
{
  B a;
  Proto<A, B> p;
  std::vector<char> buff;
  OStrm ost(buff);
  p.ser(ost, a);
  IStrm ist{buff.data(), buff.data() + buff.size()};
  auto isRecvdA{false};
  auto isRecvdB{false};
  p.deser(ist,
          overloaded{[&isRecvdA](const A &) { isRecvdA = true; },
                     [&isRecvdB](const B &) { isRecvdB = true; }});
  REQUIRE(!isRecvdA);
  REQUIRE(isRecvdB);
}

TEST_CASE("Version", "[proto]")
{
  Proto<A> p1;
  Proto<B> p2;
  REQUIRE(p1.version() == p2.version()); // A and B have same structure
  Proto<A, B> p3;
  REQUIRE(p1.version() != p3.version());
}
