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

// MagicCache.cpp
#include "magiccache.h"

namespace facebook_test {
namespace cachelib_magiccache {

void MagicCache::init() {
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
    defaultPool_ = cache_->addPool("default", cache_->getCacheMemoryStats().ramCacheSize);
}

bool MagicCache::put(char* key, char* data, int data_length) {
    CacheKey cacheKey(key);
    auto handle = cache_->allocate(defaultPool_, cacheKey, data_length);
    if (!handle) {
        return false; // cache may fail to evict due to too many pending writes
    }
    std::memcpy(handle->getMemory(), data, data_length);
    cache_->insertOrReplace(handle);
    return true;
}

bool MagicCache::get(char* key, char** data, int& data_length) {
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

bool MagicCache::isCached(char* key) {
    CacheKey cacheKey(key);
    auto handle = cache_->find(cacheKey);
    return handle != nullptr;
}

void MagicCache::destroy() {
    cache_.reset();
}

} // namespace cachelib_magiccache
} // namespace facebook_test
