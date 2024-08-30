#include "scheduler.hpp"

namespace renderer{
    std::unique_ptr<i_Scheduler> i_Scheduler::scheduler;


    void i_Scheduler::Create(){
        scheduler.reset(new i_Scheduler());
    }

    void i_Scheduler::Destroy(){
       scheduler.reset();
    }

    i_Scheduler::i_Scheduler(){
        CreateScheduler();
    }

    i_Scheduler::~i_Scheduler(){
        enkiScheduler.WaitforAllAndShutdown();
    }
    
    void i_Scheduler::CreateScheduler(){
        enki::TaskSchedulerConfig conf;

        conf.numTaskThreadsToCreate = std::max(std::thread::hardware_concurrency(), (unsigned int)1);

        enkiScheduler.Initialize(conf);
    }
}