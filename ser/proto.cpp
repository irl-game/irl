#include "proto.hpp"
#define LTM_DESC
#include <tomcrypt.h>

namespace internal
{
  auto calcMd5Hash(const char *data, const size_t size) -> uint32_t
  {
    hash_state md5;
    md5_init(&md5);
    md5_process(&md5, (const unsigned char *)data, size);
    union
    {
      std::array<unsigned char, 16> hash;
      uint32_t res;
    } hash;
    md5_done(&md5, hash.hash.data());
    return hash.res;
  }
} // namespace internal
