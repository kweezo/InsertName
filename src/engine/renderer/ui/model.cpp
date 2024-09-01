#include "model.hpp"


namespace renderer{

    Model::Model(){}

    Model::Model(std::string path, std::string name) {
        i_Scheduler::GetScheduler().AddTaskSetToPipe(this);
    }

    Model::Model(const Model& other){

    }

    Model& Model::operator=(const Model& other){
        return *this;
    }

    Model::~Model(){

    }
    
    void Model::ExecuteRange(enki::TaskSetPartition range, uint32_t threadNum){
        throw std::runtime_error(":D");
    }

    void Model::CopyFrom(const Model& other){

    }
}