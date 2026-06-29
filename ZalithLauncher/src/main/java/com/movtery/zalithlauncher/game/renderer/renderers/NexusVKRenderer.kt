/*
 * Zalith Launcher 2 — NVKL fork
 * NexusVKRenderer — Vulkan-native renderer bridge for VulkanMod on Android
 */

package com.movtery.zalithlauncher.game.renderer.renderers

import com.movtery.zalithlauncher.game.renderer.RendererInterface

object NexusVKRenderer : RendererInterface {
    override fun getRendererId(): String = "vulkan_nexus_vk"

    override fun getUniqueIdentifier(): String = "a1b2c3d4-nexus-vk-render-0001-abcdef012345"

    override fun getRendererName(): String = "Nexus VK Render"

    override fun getRendererSummary(): String =
        "Vulkan-native renderer via libNexus_VK_Render — VulkanMod + Mali-G52 MC2 (arm64)"

    override fun getRendererEnv(): Lazy<Map<String, String>> = lazy {
        mapOf(
            "POJAV_RENDERER"               to "opengles3_desktopgl",
            "GALLIUM_DRIVER"               to "none",
            "POJAV_ZINK_PREFER_SYSTEM_DRIVER" to "0",
            "MESA_GL_VERSION_OVERRIDE"     to "4.6",
            "MESA_GLSL_VERSION_OVERRIDE"   to "460"
        )
    }

    override fun getDlopenLibrary(): Lazy<List<String>> = lazy { emptyList() }

    override fun getRendererLibrary(): String = "libNexus_VK_Render.so"

    override fun getRendererEGL(): String = "libNexus_VK_Render.so"
}
