kult :crystal_ball: <a href="https://travis-ci.org/r-lyeh/kult"><img src="https://api.travis-ci.org/r-lyeh/kult.svg?branch=master" align="right" /></a>
====

Kult is a lightweight entity/component/system library (C++11).

## Features 
- [x] Expressive.
- [x] Tiny, cross-platform, self-contained, header-only.
- [x] ZLIB/libPNG licensed.

### Showcase
```c++
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
```

### Output
```
{       name: player #1,
        desc: this is our warrior,
        pos2: (x:3.33333,y:6.66666),
        vel2: (x:2,y:4),
}
{       name: orc #1,
        pos2: (x:0,y:0),
}
{}
{}
```

### Reading
- https://github.com/sosolimited/Entity-Component-Samples

### Alternatives
- https://github.com/alecthomas/entityx
- https://github.com/dbralir/ginseng
- https://github.com/miguelmartin75/anax
- https://github.com/Nocte-/es
- https://github.com/SuperV1234/SSVEntitySystem

### Changelog
- v1.0.0 (2015/11/24): Allow external serializer; new entity methods; extra join/exclude sugars; improve dump info
- v0.0.0 (2014/05/04): Initial commit
