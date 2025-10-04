#pragma once

#include <optional>
#include <string>

namespace graphics::font_cache {

// Retrieve the cached font path if one has been resolved previously.
const std::optional<std::string>& get();

// Store a resolved font path for reuse across graphics libraries.
void set(const std::string& path);

// Clear the cached path (primarily for error handling scenarios).
void reset();

} // namespace graphics::font_cache

