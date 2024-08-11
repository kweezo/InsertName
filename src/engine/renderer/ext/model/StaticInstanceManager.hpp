//
// Created by jakob on 8/9/24.
//

#pragma once
#include <boost/container/flat_map.hpp>

#include "Model.hpp"
#include "StaticInstanceData.hpp"

namespace renderer {
    class i_StaticInstanceManager {
    public:
        static void Init();

        static void Update();

        static void Cleanup();

        static void AddInstance(const i_ModelInstanceHandleInternal &instance, const std::weak_ptr<i_Shader> &shader);


        [[nodiscard]] static VkSemaphore GetRenderFinishedSemaphore(uint32_t frameInFlight);

        static void Draw();

    private:
        static void HandleCommandBuffers();

        static void RecordCommandBuffer(const std::weak_ptr<i_Shader>& shader,
                                        const std::list<std::shared_ptr<i_StaticInstanceData>>& instanceData,
                                        i_CommandBuffer commandBuffer);


        static boost::container::flat_map<ModelHandle, std::shared_ptr<i_StaticInstanceData> > instanceData;
        static boost::container::flat_map<std::string, std::pair<std::array<i_CommandBuffer, MAX_FRAMES_IN_FLIGHT>,
            std::list<std::shared_ptr<i_StaticInstanceData> > > >
        instanceDataPerShader;

        static uint32_t currThreadIndex;

        static std::array<i_Semaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
    };
}
