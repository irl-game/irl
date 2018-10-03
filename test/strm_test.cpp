#include <catch2/catch.hpp>
#include <ser/istrm.hpp>
#include <ser/ostrm.hpp>

TEST_CASE("istrm")
{
  const char buff[] = "Hello world";
  IStrm strm(std::begin(buff), std::end(buff));
  char tmp1[2]{};
  auto res = strm.read(std::begin(tmp1), 2);
  REQUIRE(res);
  REQUIRE(tmp1[0] == 'H');
  REQUIRE(tmp1[1] == 'e');

  char tmp2[10]{};
  res = strm.read(std::begin(tmp2), 10);
  REQUIRE(res);

  res = strm.read(std::begin(tmp1), 2);
  REQUIRE(!res);
}

TEST_CASE("ostrm")
{
  const char hello[] = "Hello world";
  std::vector<char> buff;
  OStrm strm(buff);
  strm.write(hello, sizeof(hello));

  REQUIRE(sizeof(hello) == buff.size());
  REQUIRE(std::equal(std::begin(hello), std::end(hello), std::begin(buff)));
}
