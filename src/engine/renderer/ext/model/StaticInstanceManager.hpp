//
// Created by jakob on 8/9/24.
//

#pragma once
#include <boost/container/flat_map.hpp>
#include <boost/container/detail/type_traits.hpp>

#include "Model.hpp"
#include "StaticInstanceData.hpp"

namespace renderer {
    class StaticInstanceManager {
    public:

    private:

        static boost::container::flat_map<ModelHandle, std::unique_ptr<i_StaticInstanceData>> instances;
    };
}
