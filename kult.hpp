// Kult - A lightweight entity/component/system library written in C++11
// rlyeh, 2013..2015. ZLIB/libPNG licensed.

// api #1 { add<component_t>(id); get<component_t>(id); has<component_t>(id); del<component_t>(id); dump(id); }
// api #2 { entity += component; entity[component]; entity.has(component); entity -= component; id.dump(); }

// @todo: [ entity, component, system( position, velocity ), engine ].diff(), patch(), save(), load(), versioning?

#pragma once

#define KULT_VERSION "1.0.0" /* (2015/11/24) Allow external serializer; new entity methods; extra join/exclude sugars; improve dump info
#define KULT_VERSION "0.0.0" // (2014/05/04) Initial commit */

#include <algorithm>
#include <iostream> // registerme
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <functional>

#ifdef   KULT_SERIALIZER_INC
#include KULT_SERIALIZER_INC
#endif

#ifndef  KULT_SERIALIZER_FN
#define  KULT_SERIALIZER_FN(v) (v)
#endif

#ifdef _OPENMP
#include <omp.h>
#endif

#if defined(_NDEBUG) || defined(NDEBUG)
#define KULT_DEBUG(...)
#define KULT_RELEASE(...) __VA_ARGS__
#else
#define KULT_DEBUG(...)   __VA_ARGS__
#define KULT_RELEASE(...)
#endif

namespace kult {

    template<typename V>             using set = std::set<V>;   // unordered_set
    template<typename K, typename V> using map = std::map<K,V>; // unordered_map
    using type = unsigned;

    // kult::helpers

    template<typename T>
    T zero() {
        return std::pair<T,T>().first;
    }
    template<typename T>
    T zero( const T &t ) {
        return zero<T>();
    }

    template<typename T>
    T one() {
        return !zero<T>();
    }
    template<typename T>
    T one( const T &t ) {
        return one<T>();
    }

    template<typename T>
    bool is_true( const T &t ) {
        return t != zero<T>();
    }
    template<typename T>
    bool is_false( const T &t ) {
        return !is_true<T>(t);
    }

    template<typename T>
    void reset( T &t ) {
        t = zero<T>();
    }

    template<typename T>
    T &invalid() {
        static T t;
        return reset(t), t;
    }
    template<typename T>
    T &invalid( const T &t ) {
        return invalid<T>();
    }

    // kult::id

    template<typename T = type>
    T none() {
        return invalid<T>();
    }

    template<typename T = type>
    T &id() {
        static T _id = none();
        return ++_id;
    }

    // kult::entity

    // forward declarations {
    inline type purge( const type & );
    inline std::string dump( const type & );
    template<typename T> inline decltype(T::value_type) &get( const type &id );
    template<typename T> inline decltype(T::value_type) &add( const type &id );
    template<typename T> inline bool has( const type &id );
    template<typename T> inline bool del( const type &id );
    // }

    struct entity {
        static set<entity*> &all() { // all live instances are reflected here
            static set<entity*> statics;
            return statics;
        }

        type id;
        entity( const type &id_ = kult::id() ) : id(id_) {
            all().insert(this);
        }
        ~entity() {
            all().erase(this);
        }

        operator type const () const {
            return id;
        }
        template<typename component>
        decltype(component::value_type) &operator []( const component &t ) const {
            return kult::add<component>(id), kult::get<component>(id);
        }
        template<typename component>
        const entity &operator +=( const component &t ) const {
            return kult::add<component>(id), *this;
        }
        template<typename component>
        const entity &operator -=( const component &t ) const {
            return kult::del<component>(id), *this;
        }
        template<typename component>
        bool has( const component &t ) const {
            return kult::has<component>(id);
        }
        std::string dump() const {
            return kult::dump(id);
        }
        void purge() {
            kult::purge(id);
            id = none<type>();
        }
    };

    inline set<entity*> entities() {
        return entity::all();
    }

    // kult::component

    enum GROUPBY_MODE {
        JOIN = 0, MERGE = 1, EXCLUDE = 2
    };

    template<typename T>
    inline kult::set<entity> &any() {
        static kult::set<entity> entities;
        return entities;
    }
    template<int MODE>
    inline kult::set<entity> group_by( const kult::set<entity> &A, const kult::set<entity> &B ) {
        const kult::set<entity>  *tiny, *large;
        if( A.size() < B.size() ) tiny = &A, large = &B;
        else                      tiny = &B, large = &A;
        kult::set<entity> newset;  // union first, then difference, then intersection
        /**/ if (MODE == MERGE)   { newset = *large; for( auto &id : *tiny ) newset.insert(id); }
        else if (MODE == EXCLUDE) { newset = *large; for( auto &id : *tiny ) newset.erase (id); }
        else { for( auto &id : *tiny ) if( large->find(id) != large->end() ) newset.insert(id); }
        return newset;
    }

    // sugars {
    template<class T>                            kult::set< entity > join()   { return any<T>();                                  }
    template<class T, class U>                   kult::set< entity > join()   { return group_by<JOIN>( any<T>(), any<U>() );      }
    template<class T, class U, class V>          kult::set< entity > join()   { return group_by<JOIN>( any<T>(), join<U,V>() );   }
    template<class T, class U, class V, class W> kult::set< entity > join()   { return group_by<JOIN>( any<T>(), join<U,V,W>() ); }
    template<class T>                            kult::set< entity > join(const T &t)                                     { return any<T>(); }
    template<class T, class U>                   kult::set< entity > join(const T &t, const U &u)                         { return join<T,U>(); }
    template<class T, class U, class V>          kult::set< entity > join(const T &t, const U &u, const V &v)             { return join<T,U,V>(); }
    template<class T, class U, class V, class W> kult::set< entity > join(const T &t, const U &u, const V &v, const W &w) { return join<T,U,V,W>(); }
    template<class T> kult::set<entity> exclude( const kult::set<entity> &B ) { return group_by<EXCLUDE>( any<T>(), B ); }
    template<class T> kult::set<entity> exclude( const kult::set<entity> &A, const T &t ) { return group_by<EXCLUDE>( A, any<T>() ); }
    // }

    template<typename... T>
    using system = std::function<void(T...)>;

    // parallelize
#   ifdef _MSC_VER
#       define OMP_PARA_INTERNAL __pragma(omp parallel for)
#   else
#       define OMP_PARA_INTERNAL _Pragma("omp parallel for") // C99
#   endif
#   define parallelize(id, sys) while(1) { \
        const std::vector<type> vsys( sys.begin(), sys.end() ); \
        const int end = vsys.size(); int it; \
        OMP_PARA_INTERNAL for( it = 0; it < end; ++it ) { \
            auto &id = vsys[it];
            // [...]
#   define pend \
        } break; }

    template<typename T>
    kult::map< type, T > &components() {
        static kult::map< type, T > objects;
        return objects;
    }
    template<typename T>
    inline bool has( const type &id ) {
        return components<T>().find( id ) != components<T>().end();
    }
    template<typename T>
    inline decltype(T::value_type) &get( const type &id ) {
        KULT_DEBUG(
        // safe
        static decltype(T::value_type) invalid, reset;
        return has<T>(id) ? components<T>()[id].value_type : invalid = reset;
        )
        KULT_RELEASE(
        // fast
        return components<T>()[id].value_type;
        )
    }
    template<typename T>
    inline decltype(T::value_type) &add( const type &id ) {
        any<T>().insert( id );
        return components<T>()[id].value_type;
    }
    template<typename T>
    inline bool del( const type &id ) {
        add<T>(id);
        components<T>().erase( id );
        any<T>().erase( id );
        return !has<T>( id );
    }
    struct interface {
        virtual ~interface() {}
        virtual void purge( const type & ) const = 0;
        virtual void swap ( const type &,   const type & ) const = 0;
        virtual void merge( const type &,   const type & ) const = 0;
        virtual void copy ( const type &,   const type & ) const = 0;
        virtual void dump ( std::ostream &, const type & ) const = 0;
        virtual std::string name() const = 0;
        static  std::vector<const interface*> &registered() {
            static std::vector<const interface*> vector;
            return vector;
        }
    };
    template<type NAME, typename T>
    struct component : interface {
        T value_type;
        component( bool reentrant = 0 ) {
            if( !reentrant ) {
                static struct registerme {
                    registerme() {
                        interface::registered().push_back( new component(1) );
                    }
                } st;
            }
        }
        ~component() {
            auto &list = interface::registered();
            for( auto &it : list ) {
                if( it == this ) {
                    std::swap( it, list.back() );
                    list.pop_back();
                }
            }
        }

        // sugars {
        const component &operator+=( const type &id ) const {
            return add<component>(id), *this;
        }
        T &operator[]( const type &id ) const {
            KULT_DEBUG(
            return operator+=(id), get<component>(id);
            )
            KULT_RELEASE(
            return get<component>(id);
            )
        }
        // }

        virtual std::string name() const {
            return std::string { (NAME >> 24) & 0xff, (NAME >> 16) & 0xff, (NAME >> 8) & 0xff, NAME & 0xff };
        }

        virtual void purge( const type &id ) const {
            del<component>(id);
        }
        virtual void swap( const type &dst, const type &src ) const {
            KULT_DEBUG(
                // safe
                if( has<component>(dst) && has<component>(src) ) {
                    std::swap( get<component>(dst), get<component>(src) );
                }
            )
            KULT_RELEASE(
                // fast
                std::swap( get<component>(dst), get<component>(src) );
            )
        }
        virtual void merge( const type &dst, const type &src ) const {
            add<component>(dst) = get<component>(src);
        }
        virtual void copy( const type &dst, const type &src ) const {
            if( has<component>(src) ) {
                merge( dst, src );
            } else {
                purge( dst );
            }
        }
        virtual void dump( std::ostream &os, const type &id ) const {
            if( has<component>(id) ) {
                os << "\t" << name() << ": " << KULT_SERIALIZER_FN( get<component>(id) ) << ",\n";
            }
        }
        inline T &operator()( const type &id ) {
            return get<component>(id);
        }
        inline const T &operator()( const type &id ) const {
            return get<component>(id);
        }
    };

    inline std::string dump( const type &id ) {
        std::stringstream ss; ss << '{';
        for( auto &it : interface::registered() ) {
            it->dump( ss, id );
        }
        return ss << '}', ss.str();
    }
    inline type purge( const type &id ) { // clear
        for( auto &it : interface::registered() ) {
            it->purge( id );
        }
        return id;
    }
    inline type swap( const type &dst, const type &src ) {
        for( auto &it : interface::registered() ) {
            it->swap( dst, src );
        }
        return dst;
    }
    inline type merge( const type &dst, const type &src ) {
        for( auto &it : interface::registered() ) {
            it->merge( dst, src );
        }
        return dst;
    }
    inline type copy( const type &dst, const type &src ) {
        for( auto &it : interface::registered() ) {
            it->copy( dst, src );
        }
        return dst;
    }
    inline type spawn( const type &src ) {
        return copy( id(), src );
    }
    /*
    inline type restart( const type &id ) {
        return copy( id, type(id) );
    }
    inline type respawn( const type &id ) {
        return copy( id, type(id) );
    }
    */
    inline type reset( const type &id ) {
        return copy( id, none() );
    }
    // kill(id);
    // save() -> diff( zero(), *this )
    // load() -> patch( zero(), diff );
    // undo()
    // redo()
}

#ifdef KULT_BUILD_TESTS

// unittest suite {
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#define suite(...) if(printf("------ " __VA_ARGS__),puts(""),true)
#define test(...)  (errno=0,++tst,err+=!(ok=!!(__VA_ARGS__))),printf("[%s] %d %s (%s)\n",ok?" OK ":"FAIL",__LINE__,#__VA_ARGS__,strerror(errno))
unsigned tst=0,err=0,ok=atexit([]{ suite("summary"){ printf("[%s] %d tests = %d passed + %d errors\n",err?"FAIL":" OK ",tst,tst-err,err); }});
// }

using namespace kult;

// gamedev types and constants
template<typename T>
struct vec2 {
    T x, y;
    bool operator==( const vec2 &other ) const {
        return x == other.x && y == other.y;
    }
    template<class ostream>
    friend inline ostream& operator <<( ostream &os, const vec2 &self ) {
        return os << "(x=" << self.x << ",y=" << self.y << ")", os;
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

int main() {

    suite( "helper tests") {
        test( zero<bool>() == false );
        test( zero<int>() == 0 );
        test( zero<float>() == 0.f );

        int t;
        test( one(t) == 1 );
        float f;
        test( one(f) == 1.f );

        test( is_true(3) );
        test( is_false(0) );
        test( is_true(true) );
        test( is_false(false) );
        test( is_true(-1.0) );
        test( is_false(0.0) );
    }

    suite("id tests") {
        test( none() == none() );
        auto id0 = id();
        test( id0 < id() );
        test( none() < id() );
        test( ( id() = none() ) == none() );
    }

    suite( "add<component_t>(id) syntax" ) {
        // entities
        int none = 0, player = 1, enemy = 2;

        // components
        test( !has<name>(player) );
        test( !has<position>(player) );
        test( !has<coins>(enemy) );
        test( !has<health>(enemy) );

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

        test( get<health>(player) == 100 ); // :>

        test(  has<name>(player) );
        test( !has<vec2i>(player) );
        test(  has<position>(player) );
        test(  has<health>(player) );

        test( get<mana>(enemy) == 10 );
        test( get<position>(player) == zero2f );
        test( get<health>(player) == 100 );

        // systems; here we intersect a system of all elements with <name> and <position>.
        test( (join<name, position>().size() == 2) );

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

        test( get<health>(player) == 100+0 );
        test( get<health>(enemy) == 200/2 );
        test( get<coins>(player) == 200+100 );
        test( get<coins>(enemy) == 50+0 );
        test( get<mana>(player) == 4000-50 );
        test( get<mana>(enemy) == 10-50 );

        test( del<position>(player) );
        test( !has<position>(player) );
        test( del<name>(player) );
        test( !has<name>(player) );

        test( (join<name, position>().size() == 1) );
    }

    suite( "entity[component] (or component[entity]) syntax" ) {
        // entities
        kult::entity player, enemy;

        component<'name', std::string> name;
        component<'heal', size_t>      heal;
        component<'coin', size_t>      coin;
        component<'mana', size_t>      mana;
        component<'team', bool>        team;
        component<'pos2', vec2f>       pos2;

        // components
        player[name] = "Hero";
        player[pos2] = zero2f;
        player[heal] = 100;
        player[coin] = 200;
        player[mana] = 4000;
        player[team] = true;

        enemy[name] = "Orc";
        enemy[pos2] = one2f;
        enemy[heal] = 200;
        enemy[coin] = 50;
        enemy[mana] = 10;

        test( player[heal] == 100 ); // :>

        test( player.has(name) );
        test( player.has(pos2) );
        test( player.has(heal) );

        test( enemy[mana] == 10 );
        test( player[pos2] == zero2f );
        test( player[heal] == 100 );

        // systems; here we intersect a system of all elements with <name> and <pos2>.
        test( join(name, pos2).size() == 2 );

        // systems; simulate movement
        for( auto &id : join( name, pos2 ) ) {
            vec2f &pos = pos2[id];
            pos.x += 10;
            pos.y ++;
        }

        // systems; simulate a spell bomb in entities of any type
        for( auto &id : join(mana) ) {
            mana[id] -= 50;
        }

        // systems; simulate a powerup (+$100) for all players
        for( auto &id : join(name, coin, team) ) {
            coin[id] += 100;
        }

        // systems; simulate a poison (-50%HP) to all entities that are not in team (so enemies)
        for( auto &id : exclude( join(name, heal), team ) ) {
            heal[id] *= 0.5;
        }

        test( player[heal] == 100+0 );
        test( player[heal] == 100+0 );

        test( enemy[heal] == 200/2 );
        test( player[coin] == 200+100 );
        test( enemy[coin] == 50+0 );
        test( player[mana] == 4000-50 );
        test( enemy[mana] == 10-50 );

        player -= pos2;
        player -= name;

        test( !player.has(pos2) );
        test( !player.has(name) );

        test( join(name, pos2).size() == 1 );

        test( player.dump() != "{}" );
        player.purge();
        test( player.dump() == "{}" );

        test( entities().size() == 2 );
    }

    test( entities().size() == 0 );
}

#endif
