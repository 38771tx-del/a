#pragma once
#include <cstdint>
#include "../cache/sdk.hpp"

void esp_start(void);
void esp_reset(void);
void esp_run(bool enabled, bool health_visual, const WindowState& window);
