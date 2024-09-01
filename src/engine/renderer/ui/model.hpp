#pragma once

#include <vector>
#include <exception>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <jsoncpp/json/json.h>

#include <enkiTS/TaskScheduler.h>

#include "../base/scheduler.hpp"

namespace renderer{
    class Model : enki::ITaskSet{
        public:
            Model();
            Model(std::string path, std::string name);
            Model(const Model& other);
            Model& operator=(const Model& other);
            ~Model();

        private:
            void ExecuteRange(enki::TaskSetPartition range, uint32_t threadNum) override;

            void CopyFrom(const Model& other);
    };
}