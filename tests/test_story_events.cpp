#include <gtest/gtest.h>
#include "cebu/story_event.h"
#include "cebu/absurdity.h"

using namespace cebu;

TEST(StoryEventSystemTest, AddAndRetrieveEvent) {
    StoryEventSystem system;

    AbsurdityContext context{0.5, 0.3, 0.2, 0.4};
    EventID id = system.add_event("First event", 0.5, {1, 2, 3}, context);

    ASSERT_EQ(system.event_count(), 1);

    const StoryEvent& event = system.get_event(id);
    EXPECT_EQ(event.id, id);
    EXPECT_EQ(event.description, "First event");
    EXPECT_DOUBLE_EQ(event.timestamp, 0.5);
    EXPECT_EQ(event.affected_simplices.size(), 3);
    EXPECT_DOUBLE_EQ(event.impact.surprisal, 0.5);
}

TEST(StoryEventSystemTest, AddMultipleEvents) {
    StoryEventSystem system;

    AbsurdityContext ctx1{0.2, 0.1, 0.3, 0.4};
    AbsurdityContext ctx2{0.6, 0.5, 0.7, 0.8};

    EventID id1 = system.add_event("Event 1", 0.1, {1}, ctx1);
    EventID id2 = system.add_event("Event 2", 0.5, {2, 3}, ctx2);

    EXPECT_EQ(system.event_count(), 2);
    EXPECT_NE(id1, id2);
}

TEST(StoryEventSystemTest, GetEventsInRange) {
    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4};

    system.add_event("Early", 0.1, {1}, context);
    system.add_event("Middle 1", 0.3, {2}, context);
    system.add_event("Middle 2", 0.5, {3}, context);
    system.add_event("Late", 0.8, {4}, context);

    // Get middle events
    std::vector<StoryEvent> result = system.get_events_in_range(0.2, 0.6);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].description, "Middle 1");
    EXPECT_EQ(result[1].description, "Middle 2");

    // Verify sorted order
    EXPECT_LT(result[0].timestamp, result[1].timestamp);
}

TEST(StoryEventSystemTest, RemoveEvent) {
    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4};
    EventID id = system.add_event("To remove", 0.5, {1, 2, 3}, context);

    ASSERT_EQ(system.event_count(), 1);

    system.remove_event(id);
    EXPECT_EQ(system.event_count(), 0);

    // Try to get removed event should throw
    EXPECT_THROW(system.get_event(id), std::out_of_range);
}

TEST(StoryEventSystemTest, ClearAllEvents) {
    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4};
    system.add_event("Event 1", 0.1, {1}, context);
    system.add_event("Event 2", 0.2, {2}, context);
    system.add_event("Event 3", 0.3, {3}, context);

    ASSERT_EQ(system.event_count(), 3);

    system.clear();
    EXPECT_EQ(system.event_count(), 0);
    EXPECT_EQ(system.get_all_events().size(), 0);
}

TEST(StoryEventSystemTest, InvalidTimestamp) {
    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4};

    // Negative timestamp should throw
    EXPECT_THROW(
        system.add_event("Invalid", -0.5, {1}, context),
        std::invalid_argument
    );
}

TEST(StoryEventSystemTest, InvalidRange) {
    StoryEventSystem system;

    AbsurdityContext context{0.1, 0.2, 0.3, 0.4};
    system.add_event("Valid", 0.5, {1}, context);

    // Start > end should throw
    EXPECT_THROW(
        system.get_events_in_range(1.0, 0.5),
        std::invalid_argument
    );
}

TEST(StoryEventSystemTest, GetNonExistentEvent) {
    StoryEventSystem system;

    // Get non-existent event should throw
    EXPECT_THROW(
        system.get_event(999),
        std::out_of_range
    );
}

TEST(StoryEventSystemTest, RemoveNonExistentEvent) {
    StoryEventSystem system;

    // Remove non-existent event should throw
    EXPECT_THROW(
        system.remove_event(999),
        std::out_of_range
    );
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
