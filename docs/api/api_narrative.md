# Narrative Features API

This document provides a comprehensive reference for Cebu's narrative-driven topology API, including timelines, story events, and narrative context.

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Timeline](#timeline)
3. [Story Event](#story-event)
4. [Narrative Context](#narrative-context)
5. [Milestone](#milestone)
6. [Advanced Features](#advanced-features)
7. [Code Examples](#code-examples)

---

## Core Concepts

### Narrative Components

| Class | Description | Key Features |
|-------|-------------|-------------|
| `Timeline` | Manages time progression and events | Time management, milestones, event scheduling |
| `StoryEvent` | Triggers topology changes at specific times | Conditional triggers, recurring events |
| `NarrativeContext` | Stores narrative state and metadata | Key-value store, type safety |
| `Milestone` | Marks significant points in the timeline | Time-based markers, descriptions |

---

## Timeline

### Class Definition

```cpp
class Timeline {
public:
    // Constructors
    Timeline();
    Timeline(double start_time, double end_time);
    
    // Time management
    void set_time(double time);
    double get_time() const;
    void advance(double delta);
    
    // Event management
    void add_event(const StoryEvent& event);
    void remove_event(const StoryEvent& event);
    void update();
    
    // Milestone management
    void add_milestone(double time, const std::string& description);
    void remove_milestone(double time);
    bool at_milestone() const;
    const Milestone& get_current_milestone() const;
    std::optional<Milestone> get_next_milestone() const;
    
    // Timeline properties
    double get_start_time() const;
    double get_end_time() const;
    void set_end_time(double end_time);
    
    // Events
    Event<double, double> on_time_change;  // (old_time, new_time)
    Event<const Milestone&> on_milestone_reached;  // (milestone)
};
```

### Constructor Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `start_time` | Start time of the timeline | `0.0` |
| `end_time` | End time of the timeline | `100.0` |

### Methods

#### `set_time`

```cpp
void set_time(double time);
```

Sets the current time of the timeline.

**Parameters:**
- `time`: New current time

#### `get_time`

```cpp
double get_time() const;
```

Gets the current time of the timeline.

**Returns:**
- Current time

#### `advance`

```cpp
void advance(double delta);
```

Advances the timeline by the given delta.

**Parameters:**
- `delta`: Amount of time to advance

#### `add_event`

```cpp
void add_event(const StoryEvent& event);
```

Adds a story event to the timeline.

**Parameters:**
- `event`: Story event to add

#### `update`

```cpp
void update();
```

Updates the timeline and triggers any events that should fire at the current time.

#### `add_milestone`

```cpp
void add_milestone(double time, const std::string& description);
```

Adds a milestone to the timeline.

**Parameters:**
- `time`: Time of the milestone
- `description`: Description of the milestone

#### `at_milestone`

```cpp
bool at_milestone() const;
```

Checks if the current time is at a milestone.

**Returns:**
- `true` if at a milestone, `false` otherwise

---

## Story Event

### Class Definition

```cpp
class StoryEvent {
public:
    // Constructors
    StoryEvent(double time, const std::string& name);
    
    // Properties
    double get_time() const;
    void set_time(double time);
    
    const std::string& get_name() const;
    void set_name(const std::string& name);
    
    // Conditions
    using Condition = std::function<bool()>;
    void set_condition(Condition condition);
    bool check_condition() const;
    
    // Recurrence
    void set_recurrence_interval(double interval);
    double get_recurrence_interval() const;
    bool is_recurring() const;
    
    // Events
    Event<double> on_trigger;  // (time)
    Event<double> on_condition_failed;  // (time)
    
    // Internal use
    bool should_trigger(double current_time) const;
    void trigger(double current_time);
    void reset();
};
```

### Constructor Parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `time` | Time when the event should trigger | N/A |
| `name` | Name of the event | N/A |

### Methods

#### `set_condition`

```cpp
void set_condition(Condition condition);
```

Sets a condition for the event to trigger.

**Parameters:**
- `condition`: Function that returns `true` if the event should trigger

#### `check_condition`

```cpp
bool check_condition() const;
```

Checks if the event's condition is met.

**Returns:**
- `true` if the condition is met, `false` otherwise

#### `set_recurrence_interval`

```cpp
void set_recurrence_interval(double interval);
```

Sets the recurrence interval for the event.

**Parameters:**
- `interval`: Time between recurring triggers

#### `is_recurring`

```cpp
bool is_recurring() const;
```

Checks if the event is recurring.

**Returns:**
- `true` if the event is recurring, `false` otherwise

---

## Narrative Context

### Class Definition

```cpp
class NarrativeContext {
public:
    // Value storage
    template<typename T>
    void set_value(const std::string& key, const T& value);
    
    template<typename T>
    T get_value(const std::string& key) const;
    
    template<typename T>
    T get_value(const std::string& key, const T& default_value) const;
    
    bool has_value(const std::string& key) const;
    void remove_value(const std::string& key);
    void clear();
    
    // Events
    Event<const std::string&, const Any&, const Any&> on_value_changed;  // (key, old_value, new_value)
    
private:
    std::unordered_map<std::string, Any> values_;
};
```

### Methods

#### `set_value`

```cpp
template<typename T>
void set_value(const std::string& key, const T& value);
```

Sets a value in the context.

**Parameters:**
- `key`: Key for the value
- `value`: Value to store

#### `get_value`

```cpp
template<typename T>
T get_value(const std::string& key) const;
```

Gets a value from the context.

**Parameters:**
- `key`: Key for the value

**Returns:**
- Value with the given key

**Throws:**
- `std::out_of_range` if the key doesn't exist
- `std::bad_cast` if the type doesn't match

#### `get_value` (with default)

```cpp
template<typename T>
T get_value(const std::string& key, const T& default_value) const;
```

Gets a value from the context, or returns a default value if the key doesn't exist.

**Parameters:**
- `key`: Key for the value
- `default_value`: Default value to return if key doesn't exist

**Returns:**
- Value with the given key, or default value

#### `has_value`

```cpp
bool has_value(const std::string& key) const;
```

Checks if a value exists in the context.

**Parameters:**
- `key`: Key to check

**Returns:**
- `true` if the key exists, `false` otherwise

---

## Milestone

### Struct Definition

```cpp
struct Milestone {
    double time;
    std::string description;
    
    bool operator<(const Milestone& other) const {
        return time < other.time;
    }
};
```

### Members

| Member | Description |
|--------|-------------|
| `time` | Time of the milestone |
| `description` | Description of the milestone |

---

## Advanced Features

### Event System

The narrative system uses Cebu's event system for notifications:

```cpp
// Example: Subscribe to timeline events
timeline.on_time_change.connect([](double old_time, double new_time) {
    std::cout << "Time changed from " << old_time << " to " << new_time << std::endl;
});

timeline.on_milestone_reached.connect([](const cebu::Milestone& milestone) {
    std::cout << "Reached milestone: " << milestone.description << " at time " << milestone.time << std::endl;
});

// Example: Subscribe to story event events
event.on_trigger.connect([](double time) {
    std::cout << "Event triggered at time " << time << std::endl;
});

event.on_condition_failed.connect([](double time) {
    std::cout << "Event condition failed at time " << time << std::endl;
});

// Example: Subscribe to context changes
context.on_value_changed.connect([](const std::string& key, const cebu::Any& old_value, const cebu::Any& new_value) {
    std::cout << "Value changed: " << key << std::endl;
});
```

### Type Safety

The `NarrativeContext` uses a type-safe interface with templates:

```cpp
// Set values of different types
context.set_value("health", 100);          // int
context.set_value("name", "Player");        // string
context.set_value("position", Point{0,0,0});  // custom type
context.set_value("alive", true);           // bool

// Get values with type safety
int health = context.get_value<int>("health");
std::string name = context.get_value<std::string>("name");
Point position = context.get_value<Point>("position");
bool alive = context.get_value<bool>("alive");

// Get with default value
int score = context.get_value<int>("score", 0);  // Returns 0 if score doesn't exist
```

### Recurring Events

Create events that repeat at regular intervals:

```cpp
// Create a recurring event
cebu::StoryEvent heartbeat(1.0, "Heartbeat");
heartbeat.set_recurrence_interval(1.0);  // Repeat every second

heartbeat.on_trigger.connect([&context](double time) {
    // Update heart rate
    int heart_rate = context.get_value<int>("heart_rate", 75);
    context.set_value("heart_rate", heart_rate + 1);
    std::cout << "Heartbeat at time " << time << std::endl;
});

timeline.add_event(heartbeat);
```

### Conditional Events

Create events that only trigger when conditions are met:

```cpp
// Create a conditional event
cebu::StoryEvent boss_fight(5.0, "Boss Fight");
boss_fight.set_condition([&context]() {
    // Check if player has enough level
    return context.get_value<int>("player_level", 0) >= 5;
});

boss_fight.on_trigger.connect([](double time) {
    std::cout << "Boss fight started at time " << time << std::endl;
});

boss_fight.on_condition_failed.connect([](double time) {
    std::cout << "Not enough level for boss fight at time " << time << std::endl;
});

timeline.add_event(boss_fight);
```

---

## Code Examples

### Basic Timeline Usage

```cpp
#include "cebu/narrative/timeline.h"
#include "cebu/narrative/story_event.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create timeline
    Timeline timeline(0.0, 10.0);
    
    // Add milestone
    timeline.add_milestone(5.0, "Midpoint");
    
    // Subscribe to events
    timeline.on_time_change.connect([](double old, double now) {
        std::cout << "Time: " << now << std::endl;
    });
    
    timeline.on_milestone_reached.connect([](const Milestone& m) {
        std::cout << "Milestone: " << m.description << " at " << m.time << std::endl;
    });
    
    // Create story event
    StoryEvent event(3.0, "Test Event");
    event.on_trigger.connect([](double time) {
        std::cout << "Event triggered at " << time << std::endl;
    });
    
    timeline.add_event(event);
    
    // Run timeline
    while (timeline.get_time() < timeline.get_end_time()) {
        timeline.advance(1.0);
        timeline.update();
    }
    
    return 0;
}
```

### Narrative Context Usage

```cpp
#include "cebu/narrative/timeline.h"
#include "cebu/narrative/story_event.h"
#include "cebu/narrative/narrative_context.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create timeline and context
    Timeline timeline(0.0, 10.0);
    NarrativeContext context;
    
    // Set initial values
    context.set_value("player_health", 100);
    context.set_value("score", 0);
    
    // Subscribe to context changes
    context.on_value_changed.connect([](const std::string& key, const Any& old, const Any& new_val) {
        std::cout << "Value changed: " << key << std::endl;
    });
    
    // Create event that modifies context
    StoryEvent health_event(2.0, "Health Event");
    health_event.on_trigger.connect([&context](double time) {
        int health = context.get_value<int>("player_health");
        context.set_value("player_health", health - 10);
        std::cout << "Health decreased to " << context.get_value<int>("player_health") << std::endl;
    });
    
    // Create event with condition
    StoryEvent score_event(5.0, "Score Event");
    score_event.set_condition([&context]() {
        return context.get_value<int>("player_health") > 50;
    });
    score_event.on_trigger.connect([&context](double time) {
        int score = context.get_value<int>("score");
        context.set_value("score", score + 100);
        std::cout << "Score increased to " << context.get_value<int>("score") << std::endl;
    });
    
    timeline.add_event(health_event);
    timeline.add_event(score_event);
    
    // Run timeline
    while (timeline.get_time() < timeline.get_end_time()) {
        timeline.advance(1.0);
        timeline.update();
    }
    
    // Print final state
    std::cout << "Final health: " << context.get_value<int>("player_health") << std::endl;
    std::cout << "Final score: " << context.get_value<int>("score") << std::endl;
    
    return 0;
}
```

### Complex Narrative Example

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/narrative/timeline.h"
#include "cebu/narrative/story_event.h"
#include "cebu/narrative/narrative_context.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create simplicial complex
    SimplicialComplex sc;
    
    // Create timeline and context
    Timeline timeline(0.0, 20.0);
    NarrativeContext context;
    
    // Set initial state
    context.set_value("area_unlocked", false);
    context.set_value("enemies_defeated", 0);
    
    // Event 1: Add initial area
    StoryEvent start_event(0.0, "Start");
    start_event.on_trigger.connect([&sc](double time) {
        std::cout << "Starting level at time " << time << std::endl;
        // Add initial area
        auto v0 = sc.add_vertex();
        auto v1 = sc.add_vertex();
        auto v2 = sc.add_vertex();
        sc.add_triangle(v0, v1, v2);
        std::cout << "Added initial area" << std::endl;
    });
    
    // Event 2: Unlock new area
    StoryEvent unlock_event(5.0, "Unlock Area");
    unlock_event.on_trigger.connect([&sc, &context](double time) {
        std::cout << "Unlocking new area at time " << time << std::endl;
        // Add new area
        auto v3 = sc.add_vertex();
        auto v4 = sc.add_vertex();
        auto v5 = sc.add_vertex();
        sc.add_triangle(v3, v4, v5);
        context.set_value("area_unlocked", true);
        std::cout << "Added new area" << std::endl;
    });
    
    // Event 3: Boss fight (conditional)
    StoryEvent boss_event(10.0, "Boss Fight");
    boss_event.set_condition([&context]() {
        return context.get_value<bool>("area_unlocked") && 
               context.get_value<int>("enemies_defeated") >= 3;
    });
    boss_event.on_trigger.connect([&sc](double time) {
        std::cout << "Boss fight started at time " << time << std::endl;
        // Add boss arena
        auto v6 = sc.add_vertex();
        auto v7 = sc.add_vertex();
        auto v8 = sc.add_vertex();
        auto v9 = sc.add_vertex();
        sc.add_tetrahedron(v6, v7, v8, v9);
        std::cout << "Added boss arena" << std::endl;
    });
    
    // Event 4: Victory
    StoryEvent victory_event(15.0, "Victory");
    victory_event.set_condition([&context]() {
        return context.get_value<int>("enemies_defeated") >= 5;
    });
    victory_event.on_trigger.connect([](double time) {
        std::cout << "Victory at time " << time << std::endl;
    });
    
    // Add events to timeline
    timeline.add_event(start_event);
    timeline.add_event(unlock_event);
    timeline.add_event(boss_event);
    timeline.add_event(victory_event);
    
    // Simulate gameplay
    timeline.advance(5.0);
    timeline.update();
    
    // Defeat enemies
    context.set_value("enemies_defeated", 3);
    
    timeline.advance(5.0);
    timeline.update();
    
    // Defeat more enemies
    context.set_value("enemies_defeated", 5);
    
    timeline.advance(5.0);
    timeline.update();
    
    timeline.advance(5.0);
    timeline.update();
    
    // Print final state
    std::cout << "\nFinal statistics:" << std::endl;
    std::cout << "Vertices: " << sc.vertex_count() << std::endl;
    std::cout << "Triangles: " << sc.simplex_count(2) << std::endl;
    std::cout << "Tetrahedra: " << sc.simplex_count(3) << std::endl;
    
    return 0;
}
```

---

**Cebu Version**: 0.8.0
**Last Updated**: 2026-03-18