//! [include topic]
#include "topic_data.h"
//! [include topic]

//! [include sig watcher]
#include "iox/signal_watcher.hpp"
//! [include sig watcher]

//! [include]
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
//! [include]

#include <iostream>

void publisher()
{
        //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-publisher-helloworld";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [create publisher]
    iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
    //! [create publisher]

    double ct = 0.0;
    //! [wait for term]
    while (!iox::hasTerminationRequested())
    //! [wait for term]
    {
        ++ct;

        // Retrieve a sample from shared memory
        //! [loan]
        auto loanResult = publisher.loan();
        //! [loan]
        //! [publish]
        if (loanResult.has_value())
        {
            auto& sample = loanResult.value();
            // Sample can be held until ready to publish
            sample->x = ct;
            sample->y = ct+1;
            sample->z = ct+2;
            sample.publish();
        }
        //! [publish]
        //! [error]
        else
        {
            auto error = loanResult.error();
            // Do something with error
            std::cerr << "Unable to loan sample, error code: " << error << std::endl;
        }
        //! [error]

        //! [msg]
         std::cout << APP_NAME << " got value: " << ct << "  " <<
                                                    ct+1 <<"  " <<
                                                    ct+2 << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        //! [msg]
    }
}

void subscriber()
{
       //! [initialize runtime]
    constexpr char APP_NAME[] = "iox-cpp-subscriber-helloworld";
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);
    //! [initialize runtime]

    //! [initialize subscriber]
    iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
    //! [initialize subscriber]

    // run until interrupted by Ctrl-C
    while (!iox::hasTerminationRequested())
    {
        //! [receive]
        auto takeResult = subscriber.take();
        if (takeResult.has_value())
        {
            std::cout << APP_NAME << " got value: " << takeResult.value()->x << "  " <<
                                                        takeResult.value()->y <<"  " <<
                                                        takeResult.value()->z << std::endl;
        }
        //! [receive]
        else
        {
            //! [error]
            if (takeResult.error() == iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
            {
                //std::cout << "No chunk available." << std::endl;   没数据不打印
            }
            else
            {
                std::cout << "Error receiving chunk." << std::endl;
            }
            //! [error]
        }

        //! [wait]
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        //! [wait]
    }

}



 int main()
 {

    iox::log::Logger::init(iox::log::LogLevel::INFO);

    std::thread publisherThread(publisher), subscriberThread(subscriber);

    publisherThread.join();
    subscriberThread.join();
   
     std::cout << "Finished" << std::endl;

 }


