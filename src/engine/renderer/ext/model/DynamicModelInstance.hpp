#pragma once

#include <memory>
#include <array>
#include <list>

namespace renderer{

class _DynamicModelInstance;

struct _TreeNode{
    std::list<std::weak_ptr<_DynamicModelInstance>> instances;

    std::array<_TreeNode, 4> subdivisions;
};

class _DynamicModelInstance{

};

}