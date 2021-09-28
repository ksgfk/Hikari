#pragma once

//HIKARI_EXPORT宏，用于导出符号
#if defined(HIKARI_EXPORT)
#error "multiple define HIKARI_EXPORT!"
#else
#if defined(HIKARI_SHARED)
#if defined(_MSC_VER)
#define HIKARI_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define HIKARI_EXPORT __attribute__((visibility("default")))
#else
#error "can not export symbol"
#endif
#else
#define HIKARI_EXPORT
#endif  // defined(HIKARI_SHARED)
#endif  // defined(HIKARI_EXPORT)

namespace Hikari {
}  // namespace Hikari