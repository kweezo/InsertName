#pragma once

#include <memory>
#include <thread>

#include <enkiTS/TaskScheduler.h>

namespace renderer{
    class i_Scheduler{
        public:
            static void Create();
            static void Destroy();

            ~i_Scheduler();
        private:
            i_Scheduler();
            i_Scheduler(const i_Scheduler& other) = delete;
            i_Scheduler& operator=(const i_Scheduler& other) = delete;

        static std::unique_ptr<i_Scheduler> scheduler;

        void CreateScheduler();

        enki::TaskScheduler enkiScheduler;
    };
}