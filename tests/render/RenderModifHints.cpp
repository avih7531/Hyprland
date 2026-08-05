#include <render/types.hpp>

#include <cstddef>
#include <gtest/gtest.h>

using namespace Render;
using namespace Hyprutils::Math;

static SRenderModifData focusScale(const Vector2D& center, float scale) {
    SRenderModifData data;
    data.modifs.emplace_back(SRenderModifData::RMOD_TYPE_TRANSLATE, -center);
    data.modifs.emplace_back(SRenderModifData::RMOD_TYPE_SCALE, scale);
    data.modifs.emplace_back(SRenderModifData::RMOD_TYPE_TRANSLATE, center);
    return data;
}

static void append(SRenderModifData& stack, const SRenderModifData& extra) {
    for (const auto& modif : extra.modifs)
        stack.modifs.emplace_back(modif);
}

// Mirrors drawHints: pop only when the stack has enough entries
static bool pop(SRenderModifData& stack, size_t count) {
    if (stack.modifs.size() < count)
        return false;
    stack.modifs.erase(stack.modifs.end() - static_cast<std::ptrdiff_t>(count), stack.modifs.end());
    return true;
}

TEST(RenderModifHints, appendPopRestoresPriorStack) {
    SRenderModifData stack;
    stack.modifs.emplace_back(SRenderModifData::RMOD_TYPE_SCALE, 2.f);
    const auto FOCUS = focusScale({100, 200}, 0.8f);

    append(stack, FOCUS);
    ASSERT_EQ(stack.modifs.size(), 4u);
    ASSERT_TRUE(pop(stack, FOCUS.modifs.size()));
    ASSERT_EQ(stack.modifs.size(), 1u);
    EXPECT_FLOAT_EQ(std::any_cast<float>(stack.modifs[0].second), 2.f);
}

TEST(RenderModifHints, nestedPopIsLifo) {
    SRenderModifData stack;
    const auto       OUTER = focusScale({10, 10}, 0.5f);
    const auto       INNER = focusScale({20, 20}, 0.9f);

    append(stack, OUTER);
    append(stack, INNER);
    ASSERT_EQ(stack.modifs.size(), 6u);

    ASSERT_TRUE(pop(stack, INNER.modifs.size()));
    ASSERT_EQ(stack.modifs.size(), OUTER.modifs.size());
    EXPECT_FLOAT_EQ(std::any_cast<float>(stack.modifs[1].second), 0.5f);

    ASSERT_TRUE(pop(stack, OUTER.modifs.size()));
    EXPECT_TRUE(stack.modifs.empty());
}

TEST(RenderModifHints, popRejectsUnderflow) {
    SRenderModifData stack;
    stack.modifs.emplace_back(SRenderModifData::RMOD_TYPE_SCALE, 1.f);

    EXPECT_FALSE(pop(stack, 2));
    ASSERT_EQ(stack.modifs.size(), 1u);
    EXPECT_FLOAT_EQ(std::any_cast<float>(stack.modifs[0].second), 1.f);
}
