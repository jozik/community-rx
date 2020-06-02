
#include "gtest/gtest.h"

#include "mrsa/Activities.h"
#include "../src/ScheduleEntry.h"

using namespace crx;

using EntryPtr = std::shared_ptr<ScheduleEntry>;

TEST(ActivityTests, contains) {
    // unsigned int schedule_id_, unsigned int service_code, int crx_type, std::shared_ptr<Activity>& activity,
    //float start_time, float end_time
    ScheduleEntry entry(1, "H", 1, std::shared_ptr<Activity>(nullptr), 4.1f, 6.1f);
    ASSERT_TRUE(entry.contains(4.1));
    ASSERT_TRUE(entry.contains(5));
    ASSERT_FALSE(entry.contains(6.1));
}


TEST(ActivityTests, activities) {
    std::shared_ptr<Activity> act_1 = std::make_shared<Activity>(101, DecisionType::A, 0.5);
    std::shared_ptr<Activity> act_2 = std::make_shared<Activity>(201, DecisionType::A, 0.75);

    mrsa::Activities<ScheduleEntry> acts(0);
    acts.addActivity(std::make_shared<ScheduleEntry>(0, "H", 0, act_1, 4.1f, 6.1f));
    acts.addActivity(std::make_shared<ScheduleEntry>(0, "S", 1, act_2, 10, 12));

    EntryPtr entry = acts.findActivityAt(5);
    ASSERT_EQ(101, entry->act()->atus_code_);

    entry = acts.findActivityAt(100);
    ASSERT_EQ(mrsa::Activities<ScheduleEntry>::NULL_ACT, entry);

    entry = acts.findActivityAt(11.5f);
    ASSERT_EQ(201, entry->act()->atus_code_);
}

