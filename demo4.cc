#include <iostream>
#include "kult.hpp"

// custom type
struct vec2 {
    float x, y;

    template<typename T> friend T&operator<<( T &os, const vec2 &self ) {
        return os << "(x:" << self.x << ",y:" << self.y << ")", os;
    }
};

// entities
kult::entity player, enemy;

// components
kult::component<'name', std::string> name;
kult::component<'desc', std::string> description;
kult::component<'pos2', vec2> position;
kult::component<'vel2', vec2> velocity;

// systems
kult::system<float> movement = [&]( float dt ) {
    for( auto &entity : kult::join( position, velocity ) ) {
        entity[ position ].x += entity[ velocity ].x * dt;
        entity[ position ].y += entity[ velocity ].y * dt;
    }
};

// app
int main(int argc, const char **argv) {

    // assign properties dynamically
    player[ name ] = "player #1";
    player[ position ] = { 0, 0 };
    player[ velocity ] = { 2, 4 };
    player[ description ] = "this is our warrior";

    enemy[ name ] = "orc #1";
    enemy[ position ] = { 0, 0 };

    // simulate 100 frames
    for( int i = 0; i < 100; ++i ) {
        movement( 1/60.f );
    }

    // print status
    std::cout << player.dump() << std::endl;
    std::cout << enemy.dump() << std::endl;

    // purge entities
    player.purge();
    enemy.purge();

    // print status
    std::cout << player.dump() << std::endl;
    std::cout << enemy.dump() << std::endl;

    return 0;
}
