#include "FontCache.hpp"

namespace graphics::font_cache {

namespace {
std::optional<std::string>& cachedFontPath() {
    static std::optional<std::string> s_cachedFontPath;
    return s_cachedFontPath;
}
} // namespace

const std::optional<std::string>& get() {
    return cachedFontPath();
}

void set(const std::string& path) {
    cachedFontPath() = path;
}

void reset() {
    cachedFontPath().reset();
}

} // namespace graphics::font_cache

