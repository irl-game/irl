#include "conn.hpp"
#include <cstring>
#include <iomanip>
#include <iostream>
#include <log/log.hpp>
#include <sched/sched.hpp>
#include <sstream>
#define LTM_DESC
#include <tomcrypt.h>

template <typename Iter>
static void dump(Iter b, Iter e)
{
  std::cout << std::setbase(16);
  int cnt{0};
  std::for_each(b, e, [&cnt](auto x) {
    if (++cnt % 16 == 0)
      std::cout << std::endl;
    std::cout << std::setw(3) << (unsigned)(unsigned char)x;
  });
  std::cout << std::setbase(10) << std::endl;
}

namespace Net
{
  namespace internal
  {
    struct Conn
    {
      rsa_key rsaKey{};
      chacha_state chachaRecv{};
      chacha_state chachaSend{};
    };
  }

  Conn::Conn(Sched &sched, const RsaPublicKey &publicKey, const std::string &host, int port)
    : sched(sched), internal(std::make_unique<internal::Conn>())
  {
    importKey(publicKey.data(), publicKey.size());
    uv_tcp_init(&sched.loop, &socket);
    struct sockaddr_in dest;
    uv_ip4_addr(host.c_str(), port, &dest);
    connect.data = this;
    socket.data = this;

    uv_tcp_connect(
      &connect, &socket, (const struct sockaddr *)&dest, [](uv_connect_t *req, int status) {
        Conn *conn = static_cast<Conn *>(req->data);
        if (status < 0)
        {
          LOG("connect failed");
          if (conn->onDisconn)
            conn->onDisconn();
          return;
        }

        conn->readStart();
        conn->sendRandKey();
      });
  }

  Conn::Conn(Sched &sched,
             const RsaPrivateKey &privateKey,
             std::function<int(uv_stream_t &)> &&accept)
    : sched(sched), internal(std::make_unique<internal::Conn>())
  {
    importKey(privateKey.data(), privateKey.size());
    uv_tcp_init(&sched.loop, &socket);
    auto r = accept((uv_stream_t &)socket);
    if (r < 0)
    {
      LOG(this, "accept ", r);
      if (onDisconn)
        onDisconn();
      return;
    }
    readStart();
  }

  Conn::~Conn()
  {
    auto done{false};
    uv_shutdown_t req;
    req.data = &done;
    uv_shutdown(&req, (uv_stream_t *)&socket, [](uv_shutdown_t *req, int status) {
      if (status < 0)
        LOG("shutdown failed");
      auto done = static_cast<bool *>(req->data);
      *done = true;
    });
    while (!done)
      sched.get().process();
  }

  auto Conn::readStart() -> void
  {
    socket.data = this;
    auto err = uv_read_start((uv_stream_t *)&socket,
                             [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buff) {
                               auto conn = static_cast<Conn *>(handle->data);
                               conn->inBuff.resize(suggested_size);
                               buff->base = conn->inBuff.data();
                               buff->len = std::min(conn->inBuff.size(), suggested_size);
                             },
                             [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buff) {
                               auto conn = static_cast<Conn *>(stream->data);
                               conn->onRead(nread, buff->base);
                             });
    if (err < 0)
      LOG(this, "uv_read_start error");
  }

  auto Conn::importKey(const unsigned char *key, int keySize) -> void
  {
    static struct RngHash
    {
      RngHash()
      {
        ltc_mp = ltm_desc;
        if (register_prng(&sprng_desc) == -1)
        {
          LOG(this, "Error registering sprng");
          return;
        }
        if (register_hash(&sha1_desc) == -1)
        {
          LOG(this, "Error registering sha1");
          return;
        }
      }
    } rngHash;

    auto err = rsa_import(key, keySize, &internal->rsaKey);
    if (err != CRYPT_OK)
    {
      std::cerr << "import rsa key error: " << error_to_string(err);
      return;
    }
  }

  auto Conn::sendRandKey() -> void
  {
    auto hash_idx = find_hash("sha1");
    auto prng_idx = find_prng("sprng");
    key.emplace();
    std::string randKey = "TODO generate random key";
    std::copy(std::begin(randKey), std::end(randKey), std::begin(*key));
    outBuff.resize(1024);
    unsigned long l1 = outBuff.size();
    int err = rsa_encrypt_key(key->data(),
                              key->size(),
                              (unsigned char *)outBuff.data(),
                              &l1,
                              (unsigned char *)"irl",
                              3,
                              NULL,
                              prng_idx,
                              hash_idx,
                              &internal->rsaKey);
    if (err != CRYPT_OK)
    {
      LOG(this, "rsa_encrypt_key %s", error_to_string(err));
      return;
    }
    outBuff.resize(l1);

    req.data = this;
    uv_buf_t buffs[2];
    int32_t sz = outBuff.size();
    buffs[0].base = (char *)&sz;
    buffs[0].len = sizeof(sz);
    LOG(this, "message size", sz);
    buffs[1].base = outBuff.data();
    buffs[1].len = outBuff.size();
    uv_write(&req, (uv_stream_t *)&socket, buffs, 2, [](uv_write_t *req, int status) {
      LOG("key is sent", req->data, status);
      Conn *conn = static_cast<Conn *>(req->data);
      if (conn->onConn)
        conn->onConn();
    });
    setupChacha();
  }

  auto Conn::decryptKey() -> void
  {
    const auto hash_idx = find_hash("sha1");
    key.emplace();
    unsigned long l1 = sizeof(*key);
    int err = rsa_decrypt_key((unsigned char *)packet.data(),
                              packet.size(),
                              key->data(),
                              &l1,
                              (unsigned char *)"irl",
                              3,
                              hash_idx,
                              &err,
                              &internal->rsaKey);
    if (err != CRYPT_OK)
    {
      LOG(this, "rsa_decrypt_key", error_to_string(err));
      return;
    }
    LOG(this, "key is: ", std::string(std::begin(*key), std::end(*key)));
  }

  auto Conn::disconn() -> void
  {
    if (uv_is_closing((uv_handle_t *)&socket))
      return;
    uv_close((uv_handle_t *)&socket, [](uv_handle_t *ctx) {
      Conn *conn = static_cast<Conn *>(ctx->data);
      if (conn->onDisconn)
        conn->onDisconn();
    });
  }

  auto Conn::onRead(int nread, const char *encBuff) -> void
  {
    if (nread == UV_EOF)
    {
      if (onDisconn)
        onDisconn();
      return;
    }
    LOG(this, nread);

    char buff[4096];
    LOG(this, "hasKey", key.has_value());
    auto isDecoded{false};
    dump(encBuff, encBuff + nread);
    if (key)
    {
      auto err =
        chacha_crypt(&internal->chachaRecv, (const unsigned char *)encBuff, nread, (unsigned char *)buff);
      if (err != CRYPT_OK)
      {
        LOG(this, "Error:", err);
        disconn();
        return;
      }
      isDecoded = true;
    }
    else
      std::copy(encBuff, encBuff + nread, buff);
    dump(buff, buff + nread);

    int idx = 0;
    while (idx < nread)
    {
      LOG("idx:", idx, "nread:", nread);
      LOG("remining:", remining);
      if (remining == 0)
      {
        int32_t sz;
        if (nread - idx < static_cast<int>(sizeof(sz)))
        {
          LOG(this, "Other side of connection is misbehaving: ", nread - idx, "<", sizeof(sz));
          disconn();
          return;
        }
        if (key && !isDecoded)
        {
          auto err = chacha_crypt(&internal->chachaRecv,
                                  (const unsigned char *)(buff + idx),
                                  nread - idx,
                                  (unsigned char *)(buff + idx));
          if (err != CRYPT_OK)
          {
            LOG(this, "Error:", err);
            disconn();
            return;
          }
        }
        sz = *(int32_t *)(&buff[idx]);
        LOG(this, "Packet size:", sz);
        idx += sizeof(sz);
        if (sz > 2 * 1024 * 1024)
        {
          LOG(this, "Packed is too big:", sz);
          disconn();
          return;
        }
        if (sz < 0)
        {
          LOG(this, "Packed has negative size:", sz);
          disconn();
          return;
        }
        remining = sz;
        packet.clear();
      }
      auto tmpSz = std::min(remining, nread - idx);
      auto tmpIdx = packet.size();
      packet.resize(packet.size() + tmpSz);
      std::copy(buff + idx, buff + idx + tmpSz, packet.data() + tmpIdx);
      idx += tmpSz;
      remining -= tmpSz;
      if (remining == 0)
      {
        if (!key)
        {
          decryptKey();
          setupChacha();
          if (onConn)
            onConn();
        }
        else
        {
          if (onRecv)
            onRecv(packet.data(), packet.size());
        }
      }
    }
  }

  auto Conn::setupChacha() -> void
  {
    assert(key);
    {
      auto err = chacha_setup(&internal->chachaSend, key->data(), key->size(), 20);
      if (err != CRYPT_OK)
      {
        LOG(this, "Error:", err);
        disconn();
        return;
      }

      unsigned char nonce[12];
      for (int i = 0; i < 12; ++i)
        nonce[i] = i;

      err = chacha_ivctr32(&internal->chachaSend, nonce, 12, 0);
      if (err != CRYPT_OK)
      {
        LOG(this, "Error:", err);
        disconn();
        return;
      }
    }
    {
      auto err = chacha_setup(&internal->chachaRecv, key->data(), key->size(), 20);
      if (err != CRYPT_OK)
      {
        LOG(this, "Error:", err);
        disconn();
        return;
      }

      std::array<unsigned char, 12> nonce = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
      err = chacha_ivctr32(&internal->chachaRecv, nonce.data(), nonce.size(), 0);
      if (err != CRYPT_OK)
      {
        LOG(this, "Error:", err);
        disconn();
        return;
      }
    }
  }

  auto Conn::send(const char *buff, size_t size) -> void
  {
    int32_t sz = size;
    std::vector<char> data(sizeof(sz) + size);
    std::copy((const char *)&sz, (const char *)&sz + sizeof(sz), std::begin(data));
    std::copy(buff, buff + size, std::begin(data) + sizeof(sz));
    dump(std::begin(data), std::end(data));
    outBuff.resize(data.size());
    auto err = chacha_crypt(&internal->chachaSend,
                            (const unsigned char *)data.data(),
                            data.size(),
                            (unsigned char *)outBuff.data());
    if (err != CRYPT_OK)
    {
      LOG(this, "Error:", err);
      disconn();
      return;
    }
    uv_buf_t buffs[1];
    buffs[0].base = outBuff.data();
    buffs[0].len = outBuff.size();
    req.data = this;
    LOG(this, "Sending packet, size: ", data.size());
    dump(std::begin(outBuff), std::end(outBuff));
    uv_write(&req, (uv_stream_t *)&socket, buffs, 1, [](uv_write_t *req, int status) {
      LOG("data sent", req->data, status);
    });
  }
} // namespace Net
