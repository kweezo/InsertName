//
// Created by jakob on 8/9/24.
//

#pragma once
#include <boost/container/flat_map.hpp>

#include "Model.hpp"
#include "InstanceData.hpp"
#include "../camera/Camera.hpp"

namespace renderer {
    class i_InstanceManager {
    public:
        static void Init();

        static void EarlyUpdate();
        static void Update();

        static void Cleanup();

        static void AddInstance(const i_ModelInstanceHandleInternal &instance, const ShaderHandle &shader);


        [[nodiscard]] static VkSemaphore GetRenderFinishedSemaphore();

        static void Draw(const i_Semaphore& presentSemaphore, const i_Fence& inFlightFence);

    private:
        static void HandleCommandBuffers();
        static void UpdateDataBuffers();

        static void RecordCommandBuffer(const ShaderHandle &shader,
                                        const std::list<std::shared_ptr<i_InstanceData> > &instanceData,
                                        i_CommandBuffer commandBuffer);


        static boost::container::flat_map<std::string, std::shared_ptr<i_InstanceData> > instanceData;
        static boost::container::flat_map<std::string, std::pair<std::array<i_CommandBuffer, MAX_FRAMES_IN_FLIGHT>,
            std::list<std::shared_ptr<i_InstanceData> > > >
        instanceDataPerShader; //shitty name but I couldnt give less than 2 shits

        static uint32_t currThreadIndex;

        static std::array<i_Semaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    };
}
