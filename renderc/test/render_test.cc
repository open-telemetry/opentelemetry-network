// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <generated/test/app1/auto_handles.h>
#include <generated/test/app1/containers.inl>
#include <generated/test/app1/handles.h>
#include <generated/test/app1/index.h>
#include <generated/test/app1/modifiers.h>
#include <generated/test/metrics.h>

#include <gtest/gtest.h>

#include <unordered_map>

// Test auto handle, which hold references when they are in scope
TEST(RenderTest, AutoHandle)
{
  test::app1::Index index;

  {
    auto span = index.simple_span.alloc();
    ASSERT_TRUE(span.valid());

    ASSERT_EQ(index.simple_span.size(), 1ul);
  }

  // When the auto handle goes out of scope, the span should be freed
  ASSERT_EQ(index.simple_span.size(), 0ul);

  {
    auto span = index.simple_span.alloc();
    ASSERT_TRUE(span.valid());

    ASSERT_EQ(index.simple_span.size(), 1ul);

    // Manually put the reference
    span.put();
    // `put()` should make the handle invalid to make later accesses fail, and facilitate debugging
    ASSERT_FALSE(span.valid());

    // The span should be freed after put even though the auto handle is still in scope
    ASSERT_EQ(index.simple_span.size(), 0ul);
  }
}

// Test handles, which are used for memory-efficient reference storage
TEST(RenderTest, Handle)
{
  static constexpr u32 the_number = 42;

  test::app1::Index index;

  auto auto_handle = index.simple_span.alloc();
  ASSERT_TRUE(auto_handle.valid());

  // Set the integer field to some important number.
  auto_handle.modify().number(the_number);
  ASSERT_EQ(auto_handle.number(), the_number);

  // Convert auto-handle to handle.
  auto handle = auto_handle.to_handle();
  ASSERT_TRUE(handle.valid());

  // Auto-handle is released.
  ASSERT_FALSE(auto_handle.valid());
  
  // The conversion to handle should not release the span
  ASSERT_EQ(index.simple_span.size(), 1ul);

  // Check that it's the same span.
  ASSERT_EQ(handle.access(index).number(), the_number);

  // Put()ing the reference should invalidate the handle and free the span
  handle.put(index);
  ASSERT_FALSE(handle.valid());
  ASSERT_EQ(index.simple_span.size(), 0ul);
}

// Test moving auto-handles to handles, which is usually done when holding handles in containers.
TEST(RenderTest, MovedToHandle)
{
  static constexpr u32 the_number = 42;

  test::app1::Index index;

  std::unordered_map<u32, test::app1::handles::simple_span> handles;

  {
    auto auto_handle = index.simple_span.alloc();
    ASSERT_TRUE(auto_handle.valid());

    // Move the reference from the auto-handle into the hashmap.
    auto [it, inserted] = handles.try_emplace(the_number, std::move(auto_handle));
    ASSERT_TRUE(inserted);

    // Reference is moved-out, so this auto-handle no longer holds it.
    ASSERT_FALSE(auto_handle.valid());

    // Only one span is allocated.
    ASSERT_EQ(index.simple_span.size(), 1ul);
  }

  {
    auto auto_handle = index.simple_span.alloc();
    ASSERT_TRUE(auto_handle.valid());

    // Since this key already exists inside the hashmap, `try_emplace` will do nothing.
    auto [it, inserted] = handles.try_emplace(the_number, std::move(auto_handle));
    ASSERT_FALSE(inserted);

    // Auto-handle still holds the reference.
    ASSERT_TRUE(auto_handle.valid());

    // In total two spans are allocated.
    ASSERT_EQ(index.simple_span.size(), 2ul);
  }

  // This would fail with `test::app1::handles::simple_span::~simple_span(): Assertion `!valid()' failed.`
  //
  //{
  //  auto auto_handle = index.simple_span.alloc();
  //  handles.insert({the_number, auto_handle.to_handle()});
  //}

  // Put()-back all references in the hashmap.
  for (auto &[key, handle] : handles) {
    handle.put(index);
  }

  ASSERT_EQ(index.simple_span.size(), 0ul);
}

// Test lookup and reference counting of indexed spans
TEST(RenderTest, IndexedSpan)
{
  static constexpr u32 key = 42;

  test::app1::Index index;

  {
    // Allocate a span
    auto ahandle = index.indexed_span.by_key(key);
    ASSERT_TRUE(ahandle.valid());
    ASSERT_EQ(ahandle.number(), key);

    ASSERT_EQ(index.indexed_span.size(), 1ul);

    {
      // Get another reference to the same span using lookup
      auto another = index.indexed_span.by_key(key);
      ASSERT_TRUE(another.valid());

      // Still only one span is allocated.
      ASSERT_EQ(index.indexed_span.size(), 1ul);

      // It's the same span.
      ASSERT_EQ(ahandle.loc(), another.loc());
    }

    // The first reference is still in scope, 'another' should not free the span
    ASSERT_EQ(index.indexed_span.size(), 1ul);

    {
      auto different = index.indexed_span.by_key(key + 1);
      ASSERT_TRUE(different.valid());

      // Additional span has been allocated.
      ASSERT_EQ(index.indexed_span.size(), 2ul);

      // It's not the same span.
      ASSERT_NE(ahandle.loc(), different.loc());
    }
  }

  ASSERT_EQ(index.indexed_span.size(), 0ul);
}

// Test MetricStore updates and iteration, and its interaction with span reference counting
TEST(RenderTest, MetricStore)
{
  using metrics_visitor_t =
      std::function<void(u64, test::app1::weak_refs::metrics_span, test::metrics::some_metrics const &, u64)>;

  static constexpr u64 timeslot_duration = 1'000'000'000;
  u64 time_now = 1;

  test::app1::Index index;

  test::metrics::some_metrics_point input_metrics{
      .active = 55,
      .total = 100,
  };

  {
    auto span = index.metrics_span.alloc();
    ASSERT_TRUE(span.valid());

    ASSERT_EQ(index.metrics_span.size(), 1ul);
    ASSERT_EQ(span.refcount(), 1ul);

    span.metrics_update(time_now, input_metrics);

    ASSERT_EQ(index.metrics_span.size(), 1ul);

    // Metric store is keeping a reference to this span.
    ASSERT_EQ(span.refcount(), 2ul);
  }

  // Metric store is keeping the span allocated.
  ASSERT_EQ(index.metrics_span.size(), 1ul);

  // Metric slot should not be ready yet.
  ASSERT_FALSE(index.metrics_span.metrics_ready(time_now));

  // Advance the current time.
  time_now += 2 * timeslot_duration;

  // Metrics slot should be ready.
  ASSERT_TRUE(index.metrics_span.metrics_ready(time_now));

  // Get the metrics from the current slot.
  int metric_counter{0};
  test::metrics::some_metrics slot_metrics{0};
  metrics_visitor_t on_metric = [&metric_counter, &slot_metrics](u64 timestamp, auto span, auto metrics, u64 interval) {
    ++metric_counter;
    slot_metrics = metrics;
  };
  index.metrics_span.metrics_foreach(time_now, on_metric);

  // Only one metrics slot.
  ASSERT_EQ(metric_counter, 1);

  // Output metrics match input metrics.
  ASSERT_EQ(slot_metrics.active, input_metrics.active);
  ASSERT_EQ(slot_metrics.total, input_metrics.total);

  // Metrics store should be cleared out.
  ASSERT_TRUE(index.metrics_span.metrics.current_queue().empty());

  // Metric store should no longer keep a reference to the span.
  ASSERT_EQ(index.metrics_span.size(), 0ul);
}

TEST(RenderTest, ManualReference)
{
  test::app1::Index index;

  auto span = index.span_with_manual_reference.alloc();
  ASSERT_TRUE(span.valid());

  auto simple_loc = span.manual_reference().loc();

  // Currently the reference is an invalid reference.
  ASSERT_EQ(simple_loc, span.manual_reference().invalid);

  // No simple_span is allocated.
  ASSERT_EQ(index.simple_span.size(), 0ul);

  {
    auto s = index.simple_span.alloc();
    ASSERT_TRUE(s.valid());

    // Save the location of this newly-allocated simple_span.
    simple_loc = s.loc();

    // Assign it as the reference.
    span.modify().manual_reference(s.get());

    ASSERT_EQ(index.simple_span.size(), 1ul);
    ASSERT_EQ(span.manual_reference().refcount(), 2ul);
  }

  ASSERT_TRUE(span.manual_reference().valid());
  ASSERT_EQ(span.manual_reference().refcount(), 1ul);

  // It's the same simple_span.
  ASSERT_EQ(simple_loc, span.manual_reference().loc());
}

// Test auto references, which are recomputed whenever relevant span fields are modified
TEST(RenderTest, AutoReference)
{
  static constexpr u32 key_one = 11;
  static constexpr u32 key_two = 22;

  test::app1::Index index;

  // Allocate 'key_one' directly on the index
  auto indexed = index.indexed_span.by_key(key_one);
  ASSERT_TRUE(indexed.valid());
  ASSERT_EQ(indexed.number(), key_one);

  // Only one indexed_span exists for now (namely indexed_span{key_one}).
  ASSERT_EQ(index.indexed_span.size(), 1ul);

  auto span = index.span_with_auto_reference.alloc();
  ASSERT_TRUE(span.valid());

  // The auto-reference is not yet valid because the `number` field that is used in the reference key (see test.render)
  // has not been assigned.
  ASSERT_FALSE(span.auto_reference().valid());

  // Still only one indexed_span exists (indexed_span{key_one}).
  ASSERT_EQ(index.indexed_span.size(), 1ul);

  // Assign the field that is used in the auto-reference key.
  span.modify().number(key_two);

  // This caused the reference to be computed and a new indexed_span to be allocated (indexed_span{key_two}).
  ASSERT_EQ(index.indexed_span.size(), 2ul);

  // Now the reference is valid.
  ASSERT_TRUE(span.auto_reference().valid());

  // We have two different indexed_span instances: indexed_span{key_one} and indexed_span{key_two}.
  ASSERT_NE(indexed.loc(), span.auto_reference().loc());
  ASSERT_EQ(indexed.number(), key_one);
  ASSERT_EQ(span.auto_reference().number(), key_two);

  // Set the field that is used in the reference key to the `key_one` value.
  span.modify().number(key_one);

  // This caused the reference to be recomputed, and now the reference points to indexed_span{key_one}, while the
  // indexed_span{key_two} instance has been free'd.
  ASSERT_EQ(index.indexed_span.size(), 1ul);

  // Those two are the same indexed_span instance (indexed_span{key_one}).
  ASSERT_EQ(indexed.loc(), span.auto_reference().loc());

  // Release the handle.
  indexed.put();

  // The auto-reference is keeping the span allocated.
  ASSERT_EQ(index.indexed_span.size(), 1ul);
}

// Test cached references, which are references that are re-computed when they are accessed
TEST(RenderTest, CachedReference)
{
  static constexpr u32 key_one = 11;
  static constexpr u32 key_two = 22;

  test::app1::Index index;

  auto indexed = index.indexed_span.by_key(key_one);
  ASSERT_TRUE(indexed.valid());
  ASSERT_EQ(indexed.number(), key_one);

  // Only one indexed_span exists for now -- indexed_span{key_one}.
  ASSERT_EQ(index.indexed_span.size(), 1ul);

  auto span = index.span_with_cached_reference.alloc();
  ASSERT_TRUE(span.valid());

  // Still only one indexed_span exists.
  ASSERT_EQ(index.indexed_span.size(), 1ul);

  // Assign the field that is used in the reference key (see test.render).
  span.modify().number(key_two);

  // And still only one indexed_span exists.
  ASSERT_EQ(index.indexed_span.size(), 1ul);

  // Accessing the reference.
  ASSERT_TRUE(span.cached_reference().valid());

  // Accessing the referencing caused the indexed_span{key_two} to be allocated.
  ASSERT_EQ(index.indexed_span.size(), 2ul);

  // We have two different indexed_span instances: indexed_span{key_one} and indexed_span{key_two}.
  ASSERT_NE(indexed.loc(), span.cached_reference().loc());
  ASSERT_EQ(indexed.number(), key_one);
  ASSERT_EQ(span.cached_reference().number(), key_two);

  // Set the field that is used in the reference key to the `key_one` value.
  span.modify().number(key_one);

  // Still two are allocated -- reference will be recomputed only after it is accessed.
  ASSERT_EQ(index.indexed_span.size(), 2ul);

  // Access the reference, causing it to be recomputed.
  ASSERT_TRUE(span.cached_reference().valid());

  // We're back to there being only one indexed_span -- indexed_span{key_one}.
  ASSERT_EQ(index.indexed_span.size(), 1ul);

  // Those are two same indexed_span instances (indexed_span{key_one}).
  ASSERT_EQ(indexed.loc(), span.cached_reference().loc());

  // Release the handle.
  indexed.put();

  // The cached reference is keeping the span allocated.
  ASSERT_EQ(index.indexed_span.size(), 1ul);
}
