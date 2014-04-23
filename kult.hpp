// Kult - A lightweight entity/component library written in C++11
// rlyeh, 2013 - 2014. MIT licensed.

#pragma once

#include <algorithm>
#include <iostream> // registerme
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <omp.h>
#include <sstream>

#ifdef   KULT_DEFINE
#include <medea/medea.hpp>
namespace kult {
    using medea::print;
}
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
    T one() {
        return !zero<T>();
    }

    template<typename T>
    T zero( const T &t ) {
        return zero<T>();
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

    // kult::entity, kult::component

    enum SUBSYSTEM_MODE {
        JOIN = 0, MERGE = 1, EXCLUDE = 2
    };

    template<typename T>
    inline kult::set<type> &system() {
        static kult::set<type> entities;
        return entities;
    }
    template<typename T, int MODE>
    inline kult::set<type> subsystem( const kult::set<type> &B ) {
        const kult::set<type> &A = system<T>(), *tiny, *large;
        if( A.size() < B.size() ) tiny = &A, large = &B;
        else                      tiny = &B, large = &A;
        kult::set<type> newset;  // union first, then difference, then intersection
        /**/ if (MODE == MERGE)   { newset = *large; for( auto &id : *tiny ) newset.insert(id); }
        else if (MODE == EXCLUDE) { newset = *large; for( auto &id : *tiny ) newset.erase (id); }
#if 1
        else { for( auto &id : *tiny ) if( large->find(id) != large->end() ) newset.insert(id); }
#else
        else { std::set_union( A.begin(), A.end(), B.begin(), B.end(), std::inserter(newset, newset.begin()) ); }
#endif
        return newset;
    }

// sugars
template<class T, class U>                   kult::set< type > join() { return subsystem<T,JOIN>( system<U>() );   }
template<class T, class U, class V>          kult::set< type > join() { return subsystem<T,JOIN>( join<U,V>() );   }
template<class T, class U, class V, class W> kult::set< type > join() { return subsystem<T,JOIN>( join<U,V,W>() ); }
template<class T> kult::set< type > exclude( const kult::set<type> &B ) { return subsystem<T,EXCLUDE>(B); }

// parallelize
#ifdef _MSC_VER
#define OMP_PARA_INTERNAL __pragma(omp parallel for)
#else
#define OMP_PARA_INTERNAL _Pragma("omp parallel for") // C99
#endif
#define parallelize(id, sys) while(1) { \
const std::vector<type> vsys( sys.begin(), sys.end() ); \
const int end = vsys.size(); int it; \
OMP_PARA_INTERNAL for( it = 0; it < end; ++it ) { \
    auto &id = vsys[it];
    // [...]
#define pend \
} break; }

    template<typename T>
    kult::map< type, T > &components() {
        static kult::map< type, T > objects;
        return objects;
    }
    template<typename T>
    bool has( type id ) {
        return components<T>().find( id ) != components<T>().end();
    }
    template<typename T>
    inline decltype(T::value_type) &get( type id ) {
#if 1 // fast
        return components<T>()[id].value_type;
#else // safe
        static decltype(T::value_type) invalid, reset;
        return has<T>(id) ? components<T>()[id].value_type : invalid = reset;
#endif
    }
    template<typename T>
    decltype(T::value_type) &add( type id ) {
        system<T>().insert( id );
        components<T>()[id] = components<T>()[id];
        return get<T>(id);
    }
    template<typename T> bool del( type id ) {
        add<T>(id);
        components<T>().erase( id );
        system<T>().erase( id );
        return !has<T>( id );
    }
    struct icomponent;
    std::vector<const icomponent*>registered;
    struct icomponent {
        virtual ~icomponent() {}
        virtual void purge( type ) const = 0;
        virtual void swap( type, type ) const = 0;
        virtual void merge( type, type ) const = 0;
        virtual void copy( type, type ) const = 0;
        virtual void dump( std::ostream &, type ) const = 0;
    };
    template<type NAME, typename T>
    struct component : icomponent {
        T value_type;
        component() {
            static struct registerme {
                registerme() {
                    /*
                    char n = ( NAME >> 24 ) & 0xff;
                    char a = ( NAME >> 16 ) & 0xff;
                    char m = ( NAME >>  8 ) & 0xff;
                    char e = ( NAME >>  0 ) & 0xff;
                    std::cout << "<kult/kult.hpp> says: registering component [" << this << "] " << n << a << m << e << std::endl;
                    */
                    registered.push_back( new component() );
                }
            } _;
        }
        virtual void purge( type id ) const {
            del<component>(id);
        }
        virtual void swap( type to, type from ) const {
            //if( has<component>(to) && has<component>(from) )
            std::swap( get<component>(to), get<component>(from) );
        }
        virtual void merge( type to, type from ) const {
            add<component>(to) = get<component>(from);
        }
        virtual void copy( type to, type from ) const {
            if( has<component>(from) ) {
                merge( to, from );
            } else {
                purge( to );
            }
        }
        virtual void dump( std::ostream &os, type id ) const {
#ifdef KULT_DEFINE
            std::stringstream is;
            os << ( kult::print( get<component>(id), is ), is.str() ) << ',';
#else
            std::stringstream ss;
            ss << get<component>(id) << ',';
            os << ss.str();
#endif
        }
        inline typename T &operator()( type id ) {
            return get<component>(id);
        }
        inline typename const T &operator()( type id ) const {
            return get<component>(id);
        }
    };

    static
    std::string dump( type id ) {
        std::stringstream ss;
        for( auto &it : registered ) {
            it->dump( ss, id );
        }
        return ss.str();
    }
    static
    int purge( int id ) { // clear
        for( auto &it : registered ) {
            it->purge( id );
        }
        return id;
    }
    static
    int swap( int to, int from ) {
        for( auto &it : registered ) {
            it->swap( to, from );
        }
        return to;
    }
    static
    type merge( type to, type from ) {
        for( auto &it : registered ) {
            it->merge( to, from );
        }
        return to;
    }
    static
    type copy( type to, type from ) {
        for( auto &it : registered ) {
            it->copy( to, from );
        }
        return to;
    }
    static
    int spawn( int from ) {
        return copy( id(), from );
    }
    /*
    static
    int restart( int id ) {
        return copy( id, type(id) );
    }
    static
    int respawn( int id ) {
        return copy( id, type(id) );
    }
    */
    static
    int reset( int id ) {
        return copy( id, none() );
    }
    // kill(id);
    // save() -> diff( zero(), *this )
    // load() -> patch( zero(), diff );
    // undo()
    // redo()

}
