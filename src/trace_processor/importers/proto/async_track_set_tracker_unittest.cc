/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "src/trace_processor/importers/proto/async_track_set_tracker.h"

#include "src/trace_processor/importers/common/args_tracker.h"
#include "src/trace_processor/importers/common/global_args_tracker.h"
#include "src/trace_processor/importers/common/track_tracker.h"
#include "src/trace_processor/types/trace_processor_context.h"
#include "test/gtest_and_gmock.h"

namespace perfetto {
namespace trace_processor {

class AsyncTrackSetTrackerUnittest : public testing::Test {
 public:
  AsyncTrackSetTrackerUnittest() {
    context_.storage.reset(new TraceStorage());
    context_.global_args_tracker.reset(new GlobalArgsTracker(&context_));
    context_.args_tracker.reset(new ArgsTracker(&context_));
    context_.track_tracker.reset(new TrackTracker(&context_));
    context_.async_track_set_tracker.reset(new AsyncTrackSetTracker(&context_));

    storage_ = context_.storage.get();
    tracker_ = context_.async_track_set_tracker.get();

    unnestable_id_ = tracker_->CreateUnnestableTrackSetForTesting(
        1, storage_->InternString("test"));
    legacy_unnestable_id_ =
        tracker_->InternAndroidSet(2, storage_->InternString("test"));
  }

 protected:
  TraceStorage* storage_ = nullptr;
  AsyncTrackSetTracker* tracker_ = nullptr;

  AsyncTrackSetTracker::TrackSetId unnestable_id_;
  AsyncTrackSetTracker::TrackSetId legacy_unnestable_id_;

 private:
  TraceProcessorContext context_;
};

namespace {

TEST_F(AsyncTrackSetTrackerUnittest, Smoke) {
  auto set_id = tracker_->InternAndroidSet(1, storage_->InternString("test"));

  auto begin = tracker_->Begin(set_id, 1);
  auto end = tracker_->End(set_id, 1);

  ASSERT_EQ(begin, end);

  uint32_t row = *storage_->process_track_table().id().IndexOf(begin);
  ASSERT_EQ(storage_->process_track_table().upid()[row], 1u);
  ASSERT_EQ(storage_->process_track_table().name()[row],
            storage_->InternString("test"));
}

TEST_F(AsyncTrackSetTrackerUnittest, EndFirst) {
  auto end = tracker_->End(unnestable_id_, 1);

  uint32_t row = *storage_->process_track_table().id().IndexOf(end);
  ASSERT_EQ(storage_->process_track_table().upid()[row], 1u);
  ASSERT_EQ(storage_->process_track_table().name()[row],
            storage_->InternString("test"));
}

TEST_F(AsyncTrackSetTrackerUnittest, LegacySaturating) {
  auto begin = tracker_->Begin(legacy_unnestable_id_, 1);
  auto begin_2 = tracker_->Begin(legacy_unnestable_id_, 1);

  ASSERT_EQ(begin, begin_2);
}

TEST_F(AsyncTrackSetTrackerUnittest, Unnestable) {
  auto begin = tracker_->Begin(unnestable_id_, 1);
  auto end = tracker_->End(unnestable_id_, 1);
  auto begin_2 = tracker_->Begin(unnestable_id_, 1);

  ASSERT_EQ(begin, end);
  ASSERT_EQ(begin, begin_2);
}

TEST_F(AsyncTrackSetTrackerUnittest, UnnestableMultipleEndAfterBegin) {
  auto begin = tracker_->Begin(unnestable_id_, 1);
  auto end = tracker_->End(unnestable_id_, 1);
  auto end_2 = tracker_->End(unnestable_id_, 1);

  ASSERT_EQ(begin, end);
  ASSERT_EQ(end, end_2);
}

}  // namespace
}  // namespace trace_processor
}  // namespace perfetto
