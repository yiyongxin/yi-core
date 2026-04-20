#pragma once

// C++26 迁移：<exec/task.hpp> → <execution>，exec::task<T> → std::execution::task<T>
// IWYU pragma: begin_exports
#include <exec/task.hpp>
// IWYU pragma: end_exports

namespace yi
{

template <typename T = void>
using Task = exec::task<T>;

} // namespace yi
