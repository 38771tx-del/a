#pragma once
#include <atomic>
#include "../cache/sdk.hpp"
#include <cstddef>

extern std::atomic<bool> moving;

void goto_start(std::size_t index);
