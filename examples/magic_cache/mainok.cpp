/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "cachelib/allocator/CacheAllocator.h"
#include "folly/init/Init.h"
#include <iostream>
#include <cstring>

namespace facebook_test {
namespace cachelib_magiccache {
using namespace facebook;
using Cache = cachelib::LruAllocator; // or Lru2QAllocator, or TinyLFUAllocator
using CacheConfig = typename Cache::NvmCacheConfig;
using CacheKey = typename Cache::Key;
using CacheReadHandle = typename Cache::ReadHandle;

class MagicCache {
public:
    // 初始化接口
    void init() {
        Cache::Config lruConfig;
        CacheConfig nvmConfig;
        nvmConfig.navyConfig.setBlockSize(4096);
        nvmConfig.navyConfig.setSimpleFile("/tmp/navy_cache",
                                        1024 * 1024 *1024 /*fileSize*/,
                                        false /*truncateFile*/);
        nvmConfig.navyConfig.blockCache().setRegionSize(16 * 1024 * 1024);
        nvmConfig.navyConfig.setDeviceMetadataSize(16 * 1024 * 1024);

        lruConfig.enableNvmCache(nvmConfig);

        cache_ = std::make_unique<Cache>(lruConfig);
        defaultPool_ =
            cache_->addPool("default", cache_->getCacheMemoryStats().ramCacheSize);
    }

    // 写数据缓存接口
    bool put(char* key, char* data, int data_length) {
        CacheKey cacheKey(key);
        auto handle = cache_->allocate(defaultPool_, cacheKey, data_length);
        if (!handle) {
            return false; // cache may fail to evict due to too many pending writes
        }
        std::memcpy(handle->getMemory(), data, data_length);
        cache_->insertOrReplace(handle);
        return true;
    }

    // 读缓存接口
    bool get(char* key, char** data, int& data_length) {
        CacheKey cacheKey(key);
        auto handle = cache_->find(cacheKey);
        if (!handle) {
            return false; // cache miss
        }
        data_length = handle->getSize();
        *data = new char[data_length];
        std::memcpy(*data, handle->getMemory(), data_length);
        return true;
    }

    // 判断是否存在缓存
    bool isCached(char* key) {
        CacheKey cacheKey(key);
        auto handle = cache_->find(cacheKey);
        return handle != nullptr;
    }

    // 销毁类的接口
    void destroy() {
        cache_.reset();
    }

private:
    std::unique_ptr<Cache> cache_;
    cachelib::PoolId defaultPool_;
};

} // namespace cachelib_magiccache
} // namespace facebook_test

using namespace facebook_test::cachelib_magiccache;

int main(int argc, char** argv) {
    folly::init(&argc, &argv);

    MagicCache magic_cache;
    magic_cache.init();

    // 使用缓存接口
    {
        char key[] = "key";
        char value[] = "value";
        bool res = magic_cache.put(key, value, sizeof(value));
        std::cout << "Put result: " << res << std::endl;

        char* readValue;
        int readValueLength;
        res = magic_cache.get(key, &readValue, readValueLength);
        std::cout << "Get result: " << res << std::endl;
        if (res) {
            std::cout << "Read from cache: " << std::string(readValue, readValueLength) << std::endl;
            delete[] readValue;
        }
    }

    magic_cache.destroy();
    return 0;
}
