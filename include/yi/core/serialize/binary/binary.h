#pragma once

// 二进制序列化库统一入口
// 自定义类型只需 include 此文件，并在 yi::serialize::binary 命名空间中
// 为目标类型提供 size / read / write 三个重载即可。

// IWYU pragma: begin_exports
#include "detail/detail.h"  // IWYU pragma: keep
#include "detail/size.h"    // IWYU pragma: keep
#include "detail/reader.h"  // IWYU pragma: keep
#include "detail/writer.h"  // IWYU pragma: keep
// IWYU pragma: end_exports
