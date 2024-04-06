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
#include "folly/init/Init.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <vector>

#include "magiccache.h"

namespace fs = std::filesystem;

void loadFilesToCache(const std::string& directory, facebook_test::cachelib_magiccache::MagicCache& cache) {
    std::cout << "Starting to load files from directory: " << directory << std::endl;

    int filesLoaded = 0;
    int totalFiles = 0;

    // 遍历指定目录
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            totalFiles++;

            // 获取文件路径
            auto filePath = entry.path();
            std::string key = filePath.filename().string();

            std::cout << "Processing file: " << filePath << std::endl;

            // 打开文件
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << filePath << std::endl;
                continue;
            }

            // 读取文件内容
            auto size = file.tellg();
            std::vector<char> buffer(size);
            file.seekg(0, std::ios::beg);
            file.read(buffer.data(), size);
            file.close();

            // 将文件内容推送到MagicCache
            if (cache.put(const_cast<char*>(key.c_str()), buffer.data(), static_cast<int>(size))) {
                std::cout << "Successfully cached file: " << key << std::endl;
                filesLoaded++;
            } else {
                std::cerr << "Failed to cache file: " << key << std::endl;
            }
        }
    }

    std::cout << "Finished loading files. " << filesLoaded << " out of " << totalFiles << " files were successfully loaded into the cache." << std::endl;
}

using namespace facebook_test::cachelib_magiccache;

int main(int argc, char** argv) {
    folly::init(&argc, &argv);

    MagicCache magic_cache;
    magic_cache.init();

    loadFilesToCache("/tmp/work", magic_cache);

    

    // // 使用缓存接口
    // {
    //     char key[] = "key";
    //     char value[] = "value";
    //     bool res = magic_cache.put(key, value, sizeof(value));
    //     std::cout << "Put result: " << res << std::endl;

    //     char* readValue;
    //     int readValueLength;
    //     res = magic_cache.get(key, &readValue, readValueLength);
    //     std::cout << "Get result: " << res << std::endl;
    //     if (res) {
    //         std::cout << "Read from cache: " << std::string(readValue, readValueLength) << std::endl;
    //         delete[] readValue;
    //     }
    // }

    magic_cache.destroy();
    return 0;
}
