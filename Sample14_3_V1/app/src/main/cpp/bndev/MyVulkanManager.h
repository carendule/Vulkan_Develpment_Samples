#ifndef VULKANEXBASE_MYVULKANMANAGER_H
#define VULKANEXBASE_MYVULKANMANAGER_H
#include <android_native_app_glue.h>
#include <vector>
#include <vulkan/vulkan.h>
#include "../vksysutil/vulkan_wrapper.h"
#include <vector>
#include "mylog.h"
#include "../util/ObjObject.h"
#include "../util/Dashboard2DObject.h"
#include "ShaderQueueSuit_Shadow.h"
#include "ShaderQueueSuit_CommonTex.h"
#define FENCE_TIMEOUT 100000000
class MyVulkanManager
{
  public:
    static android_app* Android_application;
    static bool loopDrawFlag;
    static std::vector<const char *> instanceExtensionNames;
    static VkInstance instance;
    static uint32_t gpuCount;
    static  std::vector<VkPhysicalDevice> gpus;
    static uint32_t queueFamilyCount;
    static std::vector<VkQueueFamilyProperties> queueFamilyprops;
    static uint32_t queueGraphicsFamilyIndex;
    static VkQueue queueGraphics;
    static uint32_t queuePresentFamilyIndex;
    static std::vector<const char *> deviceExtensionNames;
    static VkDevice device;
    static VkCommandPool cmdPool;
    static VkCommandBuffer cmdBuffer;
    static VkCommandBufferBeginInfo cmd_buf_info;
    static VkCommandBuffer cmd_bufs[1];
    static VkSubmitInfo submit_info[1];
    static uint32_t screenWidth;
    static uint32_t screenHeight;
    static VkSurfaceKHR surface;
    static std::vector<VkFormat> formats;
    static VkSurfaceCapabilitiesKHR surfCapabilities;
    static uint32_t presentModeCount;
    static std::vector<VkPresentModeKHR> presentModes;
    static VkExtent2D swapchainExtent;
    static VkSwapchainKHR swapChain;
    static uint32_t swapchainImageCount;
    static std::vector<VkImage> swapchainImages;
    static std::vector<VkImageView> swapchainImageViews;
    static VkPhysicalDeviceMemoryProperties memoryroperties;
    static VkFormat depthFormat;
    static VkFormatProperties depthFormatProps;
    static VkImage depthImage;
    static VkDeviceMemory memDepth;
    static VkImageView depthImageView;
    static VkFormat colorFormat;
    static VkFormatProperties colorFormatProps;
    static VkImage colorImage;
    static VkDeviceMemory memColor;
    static VkImageView colorImageView;
    static VkDescriptorImageInfo colorImageInfo;
    static VkSemaphore imageAcquiredSemaphore;
    static uint32_t currentBuffer;
    static VkRenderPass renderPass;
    static VkClearValue clear_values[2];
    static VkRenderPassBeginInfo rp_begin;
    static VkFence taskFinishFence;
    static VkPresentInfoKHR present;
    static VkFramebuffer* framebuffers;
    static VkFramebuffer selfTexFramebuffer;
    static ShaderQueueSuit_Shadow* sqsCTL;
    static ShaderQueueSuit_CommonTex* sqsD2D;
    static ObjObject* objObjectPM;
    static ObjObject* CHForDraw;
    static ObjObject* carForDraw;
    static ObjObject* treeForDraw;
    static float lAngle;
    static Dashboard2DObject* d2dA;
    static void init_vulkan_instance();
    static void destroy_vulkan_instance();
    static void enumerate_vulkan_phy_devices();
    static void create_vulkan_devices();
    static void destroy_vulkan_devices();
    static void create_vulkan_CommandBuffer();
    static void destroy_vulkan_CommandBuffer();
    static void create_vulkan_swapChain();
    static void destroy_vulkan_swapChain();
    static void create_vulkan_DepthBuffer();
    static void destroy_vulkan_DepthBuffer();
    static void create_vulkan_SelfColorBuffer();
    static void destroy_vulkan_SelfColorBuffer();
    static void create_render_pass();
    static void destroy_render_pass();
    static void init_queue();
    static void create_frame_buffer();
    static void destroy_frame_buffer();
    static void createDrawableObject();
    static void destroyDrawableObject();
    static void drawSceneToTex();
    static void drawSceneToScreen();
    static void drawObject();
    static void doVulkan();
    static void init_texture();
    static void destroy_textures();
    static void initPipeline();
    static void destroyPipeline();
    static void createFence();
    static void destroyFence();
    static void initPresentInfo();
    static void initMatrixAndLight();
    static void flushUniformBufferForToTex();
    static void flushTexToDesSetForToTex();
    static void flushUniformBufferForToScreen();
    static void flushTexToDesSetForToScreen();
};
#endif 
