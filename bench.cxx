#include <map>
#include <string>
#include <sstream>
#include <iostream>

#include "kult.hpp"

// our obj components
using namespace kult;
using name    = component<'name',std::string>;
using counter = component<'cter',size_t>;

// our sample

#include <chrono>
#include <vector>
#include <iomanip>

int main( int argc, char **argv )
{
    {
        // construct an object

        //std::cout << "Sizeof(empty obj)=" << sizeof(obj) << std::endl;

        int obj1 = 1;
        add<name>(obj1) = "obj1";
        add<counter>(obj1) = 0;

        //std::cout << "Sizeof(valid obj)=" << sizeof(obj1) << std::endl;

        // benchmark its access time

        {
            std::cout << "Benchmarking get<T>(id) syntax... ";
            std::chrono::microseconds seconds1, seconds2;
            {
                auto t_start = std::chrono::high_resolution_clock::now();
                for( size_t i = 0; i < 200000000; ++i )
                    get<counter>(obj1)++;
                seconds1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t_start);
            }
            size_t &ref = get<counter>(obj1);
            {
                auto t_start = std::chrono::high_resolution_clock::now();
                for( size_t i = 0; i < 200000000; ++i )
                    ref++;
                seconds2 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t_start);
            }
            double relative_speed = double( seconds1.count() ) / seconds2.count();
            std::cout << "relative access is x" << std::fixed << std::setprecision(2) << relative_speed << " times " << ( seconds1 >= seconds2 ? "slower" : "faster" ) << " than direct access" << std::endl;
        }

        // debug obj
        std::cout << dump(obj1) << std::endl;
    }

    {
        // obj
        kult::entity obj;
        kult::component<'cter', size_t> counter;

        {
            std::cout << "Benchmarking T[id] syntax... ";
            std::chrono::microseconds seconds1, seconds2;
            {
                auto t_start = std::chrono::high_resolution_clock::now();
                for( size_t i = 0; i < 200000000; ++i )
                    counter[ obj ]++;
                seconds1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t_start);
            }
            size_t &ref = counter[ obj ];
            {
                auto t_start = std::chrono::high_resolution_clock::now();
                for( size_t i = 0; i < 200000000; ++i )
                    ref++;
                seconds2 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t_start);
            }
            double relative_speed = double( seconds1.count() ) / seconds2.count();
            std::cout << "relative access is x" << std::fixed << std::setprecision(2) << relative_speed << " times " << ( seconds1 >= seconds2 ? "slower" : "faster" ) << " than direct access" << std::endl;
        }

        // debug obj
        std::cout << dump(obj) << std::endl;

        kult::purge(obj);

    // debug obj
        std::cout << dump(obj) << std::endl;
    }

    return 0;
}
