kult
====

- A lightweight entity/component library written in C++11
- No dependencies.
- MIT licensed.

### sample
```c++
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

auto movementSystem = []( float delta ) {
    for( auto &id : join<position,velocity>() ) {
        get<position>(id).x += get<velocity>(id).x * delta;
        get<position>(id).y += get<velocity>(id).y * delta;
    }
};

int main(int argc, char **argv) {
    int player = id();
    add<position>(player) = { 0, 0 };
    add<velocity>(player) = { 2, 4 };

    for(;;) {
        movementSystem(0.0016f);
        std::cout << get<position>(player) << std::endl;
    }

    return 0;
}
```
