Vulkan SDK must be downloaded and installed:
https://vulkan.lunarg.com/sdk/home


For ImGui styles to appear properly, some changes were necessary.
The renderer uses SRGB color format for the Vulkan surfaces. ImGui uses UNORM, so the vertex shader ImGui uses was modifed as below:
https://tuket.github.io/posts/2022-11-24-imgui-gamma/#the-actual-good-fix

For multiple viewport features:
Line 1848 of imgui\backends\imgui_impl_vulkan.cpp was modified to request VK_FORMAT_B8G8R8A8_SRGB.

