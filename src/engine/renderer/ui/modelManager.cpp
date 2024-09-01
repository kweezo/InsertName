#include "modelManager.hpp"

namespace renderer{

    std::unique_ptr<ModelManager> ModelManager::modelManager;

    void ModelManager::Create(){
        modelManager.reset(new ModelManager);
    }

    void ModelManager::Destroy(){
        modelManager.reset();
    }

    ModelManager::ModelManager(){

    }

    ModelManager::~ModelManager(){

    }

    void ModelManager::LoadSegment(std::string segment){
       modelManager->LoadConfig(segment);
    }

    void ModelManager::LoadConfig(std::string segment){
        std::string path = "engine_data/config/segments/" + segment + "/models.json";
        std::ifstream stream(path);

        if(!stream.is_open()){
            throw std::runtime_error("ERROR: Failed to open config for models from level segment " + segment);
        }

        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string err;
        if(!Json::parseFromStream(builder, stream, &root, &err)){
            throw std::runtime_error("Failed to parse shaders.json: " + err);
        }


        Json::Value& models = root["models"];

        this->models.reserve(models.size());
        
        for(const auto& name : models.getMemberNames()){
           Json::Value& path = models[name]; 

            this->models[name.c_str()] = Model(path.asString(), name.c_str());
        }

        stream.close();
    }
}