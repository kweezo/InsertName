#pragma once

#include <memory>


#include <boost/container/flat_map.hpp>

#include "../../base/logicalDevice.hpp"

namespace renderer{
    class i_MemoryPoolManager{
        public:
            static void Create();
            static void Destroy();

            ~i_MemoryPoolManager();
        private:
            i_MemoryPoolManager();
            i_MemoryPoolManager(const i_MemoryPoolManager& other) = delete;
            i_MemoryPoolManager& operator=(const i_MemoryPoolManager& other) = delete;

            static std::unique_ptr<i_MemoryPoolManager> poolManager;

            std::array<std::list<VmaPool>, 0> pools;
    };
}