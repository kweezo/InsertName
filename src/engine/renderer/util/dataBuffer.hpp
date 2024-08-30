#pragma once

#include <iostream>
#include <exception>

#include <vulkan/vulkan.h>


namespace renderer{

    template<typename T>
    struct i_DataBufferCreateInfo{
        std::vector<T> data;
    };

    enum i_MemoryPriority{
        LOW = 1,
        MEDIUM = 2,
        HIGH = 3,
    };
    
    template <typename T>
    class i_DataBuffer{
        public:
            i_DataBuffer(i_DataBufferCreateInfo<T> createInfo);
            i_DataBuffer(const i_DataBuffer& other);
            i_DataBuffer& operator=(const i_DataBuffer& other);
            ~i_DataBuffer();

        private:
    };
}
