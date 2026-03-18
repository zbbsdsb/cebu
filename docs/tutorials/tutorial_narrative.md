# Narrative Features Tutorial

This tutorial will guide you through using Cebu's narrative-driven topology features, including timelines, story events, and narrative evolution.

## Table of Contents

1. [Introduction](#introduction)
2. [Setting Up](#setting-up)
3. [Timeline Management](#timeline-management)
4. [Story Events](#story-events)
5. [Narrative Evolution](#narrative-evolution)
6. [Absurdity System](#absurdity-system)
7. [Complete Example](#complete-example)

---

## Introduction

Cebu's narrative features allow you to create topology that evolves over time, driven by story events and narrative context. These features are particularly useful for:

- **Interactive storytelling**: Create topology that changes based on narrative progression
- **Game development**: Implement dynamic level design that responds to player actions
- **Simulation**: Model systems that evolve over time with changing conditions
- **Scientific visualization**: Represent data that changes over time

Key components of the narrative system include:

- **Timeline**: Manages time progression and milestones
- **Story Events**: Trigger topology changes at specific times
- **Narrative Context**: Maintains state and metadata for the narrative
- **Absurdity System**: Handles fuzzy, interval-based values for narrative metrics

---

## Setting Up

To use narrative features in Cebu, you need to include the appropriate headers and have narrative features enabled in your build.

### Required Headers

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/narrative/timeline.h"
#include "cebu/narrative/story_event.h"
#include "cebu/narrative/narrative_context.h"
#include "cebu/absurdity/absurdity_number.h"
```

### Basic Setup

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/narrative/timeline.h"
#include "cebu/narrative/story_event.h"
#include <iostream>

int main() {
    // Create a simplicial complex
    cebu::SimplicialComplex sc;
    
    // Create a timeline
    cebu::Timeline timeline;
    
    // Create a narrative context
    cebu::NarrativeContext context;
    
    // Continue with narrative setup...
    
    return 0;
}
```

---

## Timeline Management

### Creating and Configuring a Timeline

```cpp
// Create a timeline with start and end time
cebu::Timeline timeline(0.0, 100.0);  // Starts at 0, ends at 100

// Set current time
timeline.set_time(0.0);

// Get current time
double current_time = timeline.get_time();
std::cout << "Current time: " << current_time << std::endl;

// Advance time
timeline.advance(10.0);  // Move forward 10 units
std::cout << "New time: " << timeline.get_time() << std::endl;
```

### Adding Milestones

Milestones mark significant points in the timeline:

```cpp
// Add milestones
timeline.add_milestone(25.0, "Introduction");
timeline.add_milestone(50.0, "Conflict");
timeline.add_milestone(75.0, "Resolution");

// Check if current time is at a milestone
if (timeline.at_milestone()) {
    std::cout << "Reached milestone: " << timeline.get_current_milestone().description << std::endl;
}

// Get next milestone
auto next_milestone = timeline.get_next_milestone();
if (next_milestone) {
    std::cout << "Next milestone: " << next_milestone->description 
              << " at time " << next_milestone->time << std::endl;
}
```

### Timeline Events

You can subscribe to timeline events:

```cpp
// Subscribe to time change events
timeline.on_time_change.connect([](double old_time, double new_time) {
    std::cout << "Time changed from " << old_time << " to " << new_time << std::endl;
});

// Subscribe to milestone events
timeline.on_milestone_reached.connect([](const cebu::Milestone& milestone) {
    std::cout << "Reached milestone: " << milestone.description << " at time " << milestone.time << std::endl;
});
```

---

## Story Events

### Creating Story Events

Story events trigger changes in the topology at specific times:

```cpp
// Create a story event
cebu::StoryEvent event(30.0, "Add new area");

// Set event callback
event.on_trigger.connect([&sc](double time) {
    std::cout << "Event triggered at time " << time << std::endl;
    
    // Add new vertices
    auto v4 = sc.add_vertex();
    auto v5 = sc.add_vertex();
    auto v6 = sc.add_vertex();
    
    // Add new triangle
    sc.add_triangle(v4, v5, v6);
    
    std::cout << "Added new area with vertices " << v4 << ", " << v5 << ", " << v6 << std::endl;
});

// Add event to timeline
timeline.add_event(event);
```

### Event Conditions

You can add conditions to events:

```cpp
// Create a conditional event
cebu::StoryEvent conditional_event(45.0, "Unlock secret area");

// Set condition
conditional_event.set_condition([&context]() {
    // Check if player has collected all keys
    return context.get_value("keys_collected") >= 3;
});

// Set trigger callback
conditional_event.on_trigger.connect([&sc](double time) {
    std::cout << "Secret area unlocked at time " << time << std::endl;
    // Add secret area...
});

// Set condition failed callback
conditional_event.on_condition_failed.connect([](double time) {
    std::cout << "Condition failed for secret area at time " << time << std::endl;
});

timeline.add_event(conditional_event);
```

### Recurring Events

You can create recurring events:

```cpp
// Create a recurring event
cebu::StoryEvent recurring_event(10.0, "Periodic update");
recurring_event.set_recurrence_interval(5.0);  // Repeat every 5 units

recurring_event.on_trigger.connect([](double time) {
    std::cout << "Recurring event triggered at time " << time << std::endl;
});

timeline.add_event(recurring_event);
```

---

## Narrative Evolution

### Narrative Context

The narrative context stores state and metadata for the narrative:

```cpp
// Create narrative context
cebu::NarrativeContext context;

// Set values
context.set_value("player_health", 100);
context.set_value("keys_collected", 0);
context.set_value("area_unlocked", false);

// Get values
int health = context.get_value<int>("player_health");
int keys = context.get_value<int>("keys_collected");
bool area_unlocked = context.get_value<bool>("area_unlocked");

std::cout << "Health: " << health << std::endl;
std::cout << "Keys: " << keys << std::endl;
std::cout << "Area unlocked: " << (area_unlocked ? "yes" : "no") << std::endl;
```

### Context Events

You can subscribe to context changes:

```cpp
// Subscribe to value changes
context.on_value_changed.connect([](const std::string& key, const cebu::Any& old_value, const cebu::Any& new_value) {
    std::cout << "Value changed: " << key << std::endl;
    // Handle value change...
});
```

### Topology Evolution

You can use the narrative context to drive topology changes:

```cpp
// Create a story event that evolves topology based on context
cebu::StoryEvent evolution_event(50.0, "Evolve topology");

evolution_event.on_trigger.connect([&sc, &context](double time) {
    std::cout << "Evolving topology at time " << time << std::endl;
    
    // Get current state from context
    int keys = context.get_value<int>("keys_collected");
    
    if (keys >= 3) {
        // Add a special structure
        auto v7 = sc.add_vertex();
        auto v8 = sc.add_vertex();
        auto v9 = sc.add_vertex();
        auto v10 = sc.add_vertex();
        sc.add_tetrahedron(v7, v8, v9, v10);
        std::cout << "Added special structure" << std::endl;
    } else if (keys >= 1) {
        // Add a simple structure
        auto v7 = sc.add_vertex();
        auto v8 = sc.add_vertex();
        sc.add_edge(v7, v8);
        std::cout << "Added simple structure" << std::endl;
    }
});

timeline.add_event(evolution_event);
```

---

## Absurdity System

### Absurdity Numbers

The absurdity system handles fuzzy, interval-based values:

```cpp
#include "cebu/absurdity/absurdity_number.h"

// Create an absurdity number (interval-based)
cebu::AbsurdityNumber temp(20.0, 5.0);  // Mean 20, spread 5

// Get value at specific confidence
float low = temp.get_value(0.9);  // 90% confidence low bound
float high = temp.get_value(0.1);  // 90% confidence high bound
std::cout << "Temperature range (90% confidence): " << low << "-" << high << std::endl;

// Create absurdity numbers for narrative metrics
cebu::AbsurdityNumber tension(0.5, 0.2);  // Narrative tension
cebu::AbsurdityNumber excitement(0.3, 0.15);  // Narrative excitement

// Update based on events
tension += 0.2;  // Increase tension
excitement *= 1.5;  // Increase excitement

// Check if values are within certain ranges
if (tension > 0.7) {
    std::cout << "High tension!" << std::endl;
}
```

### Using Absurdity in Narrative Context

```cpp
// Store absurdity numbers in narrative context
context.set_value("tension", cebu::AbsurdityNumber(0.5, 0.2));
context.set_value("excitement", cebu::AbsurdityNumber(0.3, 0.15));

// Retrieve and update
auto tension = context.get_value<cebu::AbsurdityNumber>("tension");
tension += 0.2;
context.set_value("tension", tension);

// Use in conditions
cebu::StoryEvent climax_event(75.0, "Climax");
climax_event.set_condition([&context]() {
    auto tension = context.get_value<cebu::AbsurdityNumber>("tension");
    return tension > 0.8;  // High tension required for climax
});
```

---

## Complete Example

```cpp
#include "cebu/simplicial_complex.h"
#include "cebu/narrative/timeline.h"
#include "cebu/narrative/story_event.h"
#include "cebu/narrative/narrative_context.h"
#include "cebu/absurdity/absurdity_number.h"
#include <iostream>

int main() {
    using namespace cebu;
    
    // Create a simplicial complex
    SimplicialComplex sc;
    
    // Add initial vertices and triangle
    auto v0 = sc.add_vertex();
    auto v1 = sc.add_vertex();
    auto v2 = sc.add_vertex();
    sc.add_triangle(v0, v1, v2);
    
    std::cout << "Initial complex created with vertices " << v0 << ", " << v1 << ", " << v2 << std::endl;
    
    // Create timeline
    Timeline timeline(0.0, 100.0);
    
    // Add milestones
    timeline.add_milestone(25.0, "Introduction");
    timeline.add_milestone(50.0, "Conflict");
    timeline.add_milestone(75.0, "Resolution");
    
    // Create narrative context
    NarrativeContext context;
    context.set_value("keys_collected", 0);
    context.set_value("tension", AbsurdityNumber(0.2, 0.1));
    context.set_value("excitement", AbsurdityNumber(0.1, 0.05));
    
    // Subscribe to timeline events
    timeline.on_milestone_reached.connect([](const Milestone& milestone) {
        std::cout << "\n=== MILESTONE: " << milestone.description << " ===" << std::endl;
    });
    
    // Event 1: Introduction - add new area
    StoryEvent intro_event(10.0, "Introduction Event");
    intro_event.on_trigger.connect([&sc, &context](double time) {
        std::cout << "Introduction event triggered at time " << time << std::endl;
        
        // Add new vertices and triangle
        auto v3 = sc.add_vertex();
        auto v4 = sc.add_vertex();
        auto v5 = sc.add_vertex();
        sc.add_triangle(v3, v4, v5);
        
        // Update context
        context.set_value("keys_collected", 1);
        auto tension = context.get_value<AbsurdityNumber>("tension");
        tension += 0.1;
        context.set_value("tension", tension);
        
        std::cout << "Added new area. Keys collected: 1" << std::endl;
    });
    
    // Event 2: Conflict - add challenging area
    StoryEvent conflict_event(40.0, "Conflict Event");
    conflict_event.on_trigger.connect([&sc, &context](double time) {
        std::cout << "Conflict event triggered at time " << time << std::endl;
        
        // Add more complex structure
        auto v6 = sc.add_vertex();
        auto v7 = sc.add_vertex();
        auto v8 = sc.add_vertex();
        auto v9 = sc.add_vertex();
        sc.add_triangle(v6, v7, v8);
        sc.add_triangle(v7, v8, v9);
        
        // Update context
        context.set_value("keys_collected", 2);
        auto tension = context.get_value<AbsurdityNumber>("tension");
        tension += 0.3;
        context.set_value("tension", tension);
        auto excitement = context.get_value<AbsurdityNumber>("excitement");
        excitement += 0.2;
        context.set_value("excitement", excitement);
        
        std::cout << "Added challenging area. Keys collected: 2" << std::endl;
    });
    
    // Event 3: Climax - add final area if conditions met
    StoryEvent climax_event(60.0, "Climax Event");
    climax_event.set_condition([&context]() {
        return context.get_value<int>("keys_collected") >= 2;
    });
    climax_event.on_trigger.connect([&sc, &context](double time) {
        std::cout << "Climax event triggered at time " << time << std::endl;
        
        // Add final structure
        auto v10 = sc.add_vertex();
        auto v11 = sc.add_vertex();
        auto v12 = sc.add_vertex();
        auto v13 = sc.add_vertex();
        sc.add_tetrahedron(v10, v11, v12, v13);
        
        // Update context
        context.set_value("keys_collected", 3);
        auto tension = context.get_value<AbsurdityNumber>("tension");
        tension += 0.4;
        context.set_value("tension", tension);
        auto excitement = context.get_value<AbsurdityNumber>("excitement");
        excitement += 0.3;
        context.set_value("excitement", excitement);
        
        std::cout << "Added final area. Keys collected: 3" << std::endl;
    });
    
    // Event 4: Resolution - simplify structure
    StoryEvent resolution_event(80.0, "Resolution Event");
    resolution_event.on_trigger.connect([&sc, &context](double time) {
        std::cout << "Resolution event triggered at time " << time << std::endl;
        
        // Update context
        auto tension = context.get_value<AbsurdityNumber>("tension");
        tension -= 0.5;
        context.set_value("tension", tension);
        auto excitement = context.get_value<AbsurdityNumber>("excitement");
        excitement -= 0.3;
        context.set_value("excitement", excitement);
        
        std::cout << "Story resolved." << std::endl;
    });
    
    // Add events to timeline
    timeline.add_event(intro_event);
    timeline.add_event(conflict_event);
    timeline.add_event(climax_event);
    timeline.add_event(resolution_event);
    
    // Run the timeline
    std::cout << "\n=== RUNNING TIMELINE ===" << std::endl;
    while (timeline.get_time() < timeline.get_end_time()) {
        // Advance time
        timeline.advance(5.0);
        
        // Check for triggered events
        timeline.update();
        
        // Print current state
        std::cout << "Time: " << timeline.get_time() 
                  << ", Keys: " << context.get_value<int>("keys_collected") 
                  << ", Tension: " << context.get_value<AbsurdityNumber>("tension").get_mean() 
                  << ", Excitement: " << context.get_value<AbsurdityNumber>("excitement").get_mean() 
                  << std::endl;
    }
    
    // Final statistics
    std::cout << "\n=== FINAL STATISTICS ===" << std::endl;
    std::cout << "Vertices: " << sc.vertex_count() << std::endl;
    std::cout << "Edges: " << sc.simplex_count(1) << std::endl;
    std::cout << "Triangles: " << sc.simplex_count(2) << std::endl;
    std::cout << "Tetrahedra: " << sc.simplex_count(3) << std::endl;
    std::cout << "Final keys collected: " << context.get_value<int>("keys_collected") << std::endl;
    
    return 0;
}
```

### Expected Output

```
Initial complex created with vertices 0, 1, 2

=== RUNNING TIMELINE ===
Time: 5, Keys: 0, Tension: 0.2, Excitement: 0.1
Time: 10, Keys: 0, Tension: 0.2, Excitement: 0.1
Introduction event triggered at time 10
Added new area. Keys collected: 1
Time: 10, Keys: 1, Tension: 0.3, Excitement: 0.1
Time: 15, Keys: 1, Tension: 0.3, Excitement: 0.1
Time: 20, Keys: 1, Tension: 0.3, Excitement: 0.1
Time: 25, Keys: 1, Tension: 0.3, Excitement: 0.1

=== MILESTONE: Introduction ===
Time: 25, Keys: 1, Tension: 0.3, Excitement: 0.1
Time: 30, Keys: 1, Tension: 0.3, Excitement: 0.1
Time: 35, Keys: 1, Tension: 0.3, Excitement: 0.1
Time: 40, Keys: 1, Tension: 0.3, Excitement: 0.1
Conflict event triggered at time 40
Added challenging area. Keys collected: 2
Time: 40, Keys: 2, Tension: 0.6, Excitement: 0.3
Time: 45, Keys: 2, Tension: 0.6, Excitement: 0.3
Time: 50, Keys: 2, Tension: 0.6, Excitement: 0.3

=== MILESTONE: Conflict ===
Time: 50, Keys: 2, Tension: 0.6, Excitement: 0.3
Time: 55, Keys: 2, Tension: 0.6, Excitement: 0.3
Time: 60, Keys: 2, Tension: 0.6, Excitement: 0.3
Climax event triggered at time 60
Added final area. Keys collected: 3
Time: 60, Keys: 3, Tension: 1, Excitement: 0.6
Time: 65, Keys: 3, Tension: 1, Excitement: 0.6
Time: 70, Keys: 3, Tension: 1, Excitement: 0.6
Time: 75, Keys: 3, Tension: 1, Excitement: 0.6

=== MILESTONE: Resolution ===
Time: 75, Keys: 3, Tension: 1, Excitement: 0.6
Time: 80, Keys: 3, Tension: 1, Excitement: 0.6
Resolution event triggered at time 80
Story resolved.
Time: 80, Keys: 3, Tension: 0.5, Excitement: 0.3
Time: 85, Keys: 3, Tension: 0.5, Excitement: 0.3
Time: 90, Keys: 3, Tension: 0.5, Excitement: 0.3
Time: 95, Keys: 3, Tension: 0.5, Excitement: 0.3
Time: 100, Keys: 3, Tension: 0.5, Excitement: 0.3

=== FINAL STATISTICS ===
Vertices: 14
Edges: 21
Triangles: 7
Tetrahedra: 1
Final keys collected: 3
```

---

## Next Steps

Now that you've learned the basics of narrative features in Cebu, you can:

- Explore the [Narrative API documentation](../api/api_narrative.md) for more details
- Try using narrative features with spatial indexing
- Experiment with more complex story event sequences
- Integrate narrative features into your own projects

For more information on the absurdity system, see the [Absurdity System documentation](../advanced/absurdity_system.md).

---

**Cebu Version**: 0.8.0
**Last Updated**: 2026-03-18