#include <iostream>
#include "kult.hpp"

struct vec2 {
    float x, y;

    template<typename T> friend T&operator<<( T &os, const vec2 &self ) {
        return os << "(x:" << self.x << ",y:" << self.y << ")", os;
    }
};

using namespace kult;
using position = component<'pos2', vec2>;
using velocity = component<'vel2', vec2>;

void movementSystem( float delta ) {
    for( auto &id : join<position,velocity>() ) {
        get<position>(id).x += get<velocity>(id).x * delta;
        get<position>(id).y += get<velocity>(id).y * delta;
    }
};

int main(int argc, char **argv) {
    int player = id();
    add<position>(player) = { 0, 0 };
    add<velocity>(player) = { 2, 4 };

    for( unsigned times = 0; times < 100; ++times ) {
        movementSystem(0.0016f);
        std::cout << dump(player) << std::endl;
    }

    return 0;
}
