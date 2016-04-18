// This is the original sample from Google's ECS library: Corgi.
// I just ported the sample to Kult just for comparisons.
// - rlyeh

// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include <stdio.h>
#include <ctime>
#include "kult.hpp"

// Data types
using WorldTime = float;

// This is the Component data structure for the CounterComponent. It holds an
// `int` counter for each Entity that will be incremented on calls to `update_counters`.
struct CounterData {
    int counter;

    template<class ostream> friend inline ostream& operator <<( ostream &os, const CounterData &self ) {
        return os << "(counter=" << self.counter << ")", os;
    }
};

// The second Component is called `ScreamingComponent`. It will be a simple Component
// that stores a `std::string` representing a string literal of an Entity's `battle_cry`.
struct ScreamingData {
    std::string battle_cry;

    template<class ostream> friend inline ostream& operator <<( ostream &os, const ScreamingData &self ) {
        return os << "(battle_cry=" << self.battle_cry << ")", os;
    }
};

// Components
kult::component<'cntr', CounterData> CounterComponent;
kult::component<'scre', ScreamingData> ScreamingComponent;

// Systems
kult::system<float> update_counters = [&]( WorldTime delta_time ) {
    for( auto &entity : kult::join( CounterComponent ) ) {
        entity[ CounterComponent ].counter++;
    }
};

kult::system<float> update_entities = [&]( WorldTime delta_time ) {
    if( delta_time > 10 ) {
        for( auto &entity : kult::join( ScreamingComponent ) ) {
            puts( entity[ ScreamingComponent ].battle_cry.c_str() );
        }    
    }
};

int main() {
    // Create all of the Entities.
    kult::entity new_entity;

    // Register an Entity with all the Components it should be associated with.
    new_entity += CounterComponent;
    new_entity += ScreamingComponent;
    new_entity[ ScreamingComponent ].battle_cry = "ouch";

    // Simulate a game-loop that executes 10 times with a random delta_time.
    std::srand(static_cast<unsigned>(std::time(0)));

    for (int x = 0; x < 10; x++) {
      int mock_delta_time = std::rand() % 20 + 1;  // Random delta_time from 1-20.

      // You typically call EntityManager's `UpdateComponents` once per frame.

      // In this sample, it will first execute CounterComponent's
      // `update_counters` to increment the `counter` for each Entity registered
      // with CounterComponent.
      update_counters( mock_delta_time );

      // Next, it will execute ScreamingComponent's `update_entities` to check
      // if the `mock_delta_time` is greater than 10. If so, it will print out
      // each Entity's `battle_cry` string for each Entity that is registered with
      // ScreamingComponent.
      update_entities( mock_delta_time );
    }

    // Output the Entity's `counter` data to show that it incremented correctly.
    auto &entity_data = new_entity[ CounterComponent ];
    printf("The current counter is = %d.\n", entity_data.counter);

    // Inspect/serialization
    printf("%s\n", new_entity.dump().c_str() );
}
