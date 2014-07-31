#include <iostream>

#include "kult.hpp"

#undef  assert
#define assert(...) do { printf((__VA_ARGS__) ? "[ OK ] %s\n" : "[FAIL] %s\n", #__VA_ARGS__); } while(0)

using namespace kult;

void helper_tests() {
    assert( zero<bool>() == false );
    assert( zero<int>() == 0 );
    assert( zero<float>() == 0.f );

    int t;
    assert( one(t) == 1 );
    float f;
    assert( one(f) == 1.f );

    assert( is_true(3) );
    assert( is_false(0) );
    assert( is_true(true) );
    assert( is_false(false) );
    assert( is_true(-1.0) );
    assert( is_false(0.0) );
}

void id_tests() {
    assert( none() == none() );
    auto id0 = id();
    assert( id0 < id() );
    assert( none() < id() );
    assert( ( id() = none() ) == none() );
}

// gamedev types and constants
template<typename T>
struct vec2 {
    T x, y;
    bool operator==( const vec2 &other ) const {
        return x == other.x && y == other.y;
    }
    template<class T>
    friend inline T& operator <<( T &ostream, const vec2 &self ) {
        return ostream << "(x=" << self.x << ",y=" << self.y << ")", ostream;
    }
};

using vec2f = vec2<float>;
using vec2i = vec2<int>;

const vec2f zero2f = { 0.f, 0.f }, one2f = { 1.f, 1.f };

// component aliases
using friendly = kult::component< 'team', bool >;
using health   = kult::component< 'heal', int >;
using mana     = kult::component< 'mana', int >;
using coins    = kult::component< 'coin', int >;
using name     = kult::component< 'name', std::string >;
using position = kult::component< 'pos2', vec2f >;

// compo tests
void compo_tests() {
    // entities
    int none = 0, player = 1, enemy = 2;

    // components
    assert( !has<name>(player) );
    assert( !has<position>(player) );
    assert( !has<coins>(enemy) );
    assert( !has<health>(enemy) );

    add<name>(player) = "Hero";
    add<position>(player) = zero2f;
    add<health>(player) = 100;
    add<coins>(player) = 200;
    add<mana>(player) = 4000;
    add<friendly>(player) = true;

    add<name>(enemy) = "Orc";
    add<position>(enemy) = one2f;
    add<health>(enemy) = 200;
    add<coins>(enemy) = 50;
    add<mana>(enemy) = 10;

    assert( get<health>(player) == 100 ); // :>

    assert(  has<name>(player) );
    assert( !has<vec2i>(player) );
    assert(  has<position>(player) );
    assert(  has<health>(player) );

    assert( get<mana>(enemy) == 10 );
    assert( get<position>(player) == zero2f );
    assert( get<health>(player) == 100 );

    // systems; here we intersect a system of all elements with <name> and <position>.
    assert( (join<name, position>().size() == 2) );

    // systems; simulate movement
    for( auto &id : join<name, position>() ) {
        vec2f &pos = get<position>(id);
        pos.x += 10;
        pos.y ++;
    }

    // systems; simulate a spell bomb in entities of any type
    for( auto &id : join<mana>() ) {
        get<mana>(id) -= 50;
    }

    // systems; simulate a powerup (+$100) for all players
    for( auto &id : join<name, coins, friendly>() ) {
        get<coins>(id) += 100;
    }

    // systems; simulate a poison (-50%HP) to all entities that are not friendly (so enemies)
    for( auto &id : exclude<friendly>( join<name, health>() ) ) {
        get<health>(id) *= 0.5;
    }

    assert( get<health>(player) == 100+0 );
    assert( get<health>(enemy) == 200/2 );
    assert( get<coins>(player) == 200+100 );
    assert( get<coins>(enemy) == 50+0 );
    assert( get<mana>(player) == 4000-50 );
    assert( get<mana>(enemy) == 10-50 );

    assert( del<position>(player) );
    assert( !has<position>(player) );
    assert( del<name>(player) );
    assert( !has<name>(player) );

    assert( (join<name, position>().size() == 1) );
}

int main() {

    helper_tests();
    id_tests();
    compo_tests();

    std::cout << "All ok." << std::endl;
}
