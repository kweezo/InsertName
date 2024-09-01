#pragma once

#include <vector>
#include <exception>
#include <memory>
#include <fstream>

#include <jsoncpp/json/json.h>

#include <enkiTS/TaskScheduler.h>

#include <boost/container/flat_map.hpp>

#include "../base/scheduler.hpp"
#include "model.hpp"

namespace renderer{
    class ModelManager{
        public:
            static void Create();
            static void Destroy();

            static void LoadSegment(std::string segment);

            ~ModelManager();
        private:
            ModelManager();
            ModelManager(const ModelManager& other) = delete;
            ModelManager& operator=(const ModelManager& other) = delete;

            
            static std::unique_ptr<ModelManager> modelManager;

            boost::container::flat_map<std::string, Model> models;

            void LoadConfig(std::string segment);
    };
}
