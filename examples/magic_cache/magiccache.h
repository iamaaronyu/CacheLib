#pragma once

#include "cachelib/allocator/CacheAllocator.h"
#include <memory>
#include <cstring>

namespace facebook_test {
namespace cachelib_magiccache {

using Cache = facebook::cachelib::LruAllocator; // or Lru2QAllocator, or TinyLFUAllocator
using CacheConfig = typename Cache::NvmCacheConfig;
using CacheKey = typename Cache::Key;
using CacheReadHandle = typename Cache::ReadHandle;

class MagicCache {
public:
    void init();
    bool put(char* key, char* data, int data_length);
    bool get(char* key, char** data, int& data_length);
    bool isCached(char* key);
    void destroy();

private:
    std::unique_ptr<Cache> cache_;
    facebook::cachelib::PoolId defaultPool_;
};

} // namespace cachelib_magiccache
} // namespace facebook_test
