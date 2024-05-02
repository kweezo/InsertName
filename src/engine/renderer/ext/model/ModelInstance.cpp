#include "ModelInstance.hpp"

namespace renderer{

ModelInstance::ModelInstance(ModelHandle mode, Transform transform, bool isStatic){
    model = glm::mat4(1.0f);
    model = glm::translate(model, transform.pos);
    model = glm::scale(model, transform.scale);
    //todo, face your enemies (rotation)
}

}