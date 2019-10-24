// Copyright (C) 2019 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <random>

#include <benchmark/benchmark.h>

#include "src/trace_processor/tables/macros.h"

namespace perfetto {
namespace trace_processor {
namespace {

#define PERFETTO_TP_TEST_TABLE(NAME, PARENT, C) \
  NAME(TestTable, "test_table")                 \
  PERFETTO_TP_ROOT_TABLE(PARENT, C)

PERFETTO_TP_TABLE(PERFETTO_TP_TEST_TABLE);

}  // namespace
}  // namespace trace_processor
}  // namespace perfetto

using perfetto::trace_processor::SqlValue;
using perfetto::trace_processor::StringPool;
using perfetto::trace_processor::TestTable;

static void BM_TableInsert(benchmark::State& state) {
  StringPool pool;
  TestTable table(&pool, nullptr);

  for (auto _ : state) {
    benchmark::DoNotOptimize(table.Insert({}));
  }
}
BENCHMARK(BM_TableInsert);

static void BM_TableFilterIdColumn(benchmark::State& state) {
  StringPool pool;
  TestTable table(&pool, nullptr);

  uint32_t size = static_cast<uint32_t>(state.range(0));
  for (uint32_t i = 0; i < size; ++i)
    table.Insert({});

  for (auto _ : state) {
    benchmark::DoNotOptimize(table.Filter({table.id().eq(SqlValue::Long(30))}));
  }
}
BENCHMARK(BM_TableFilterIdColumn)
    ->RangeMultiplier(8)
    ->Range(1024, 2 * 1024 * 1024);