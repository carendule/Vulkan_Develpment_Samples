#include <vulkan/vulkan.h>
#include "MatrixState3D.h"
#include "MyVulkanManager.h"
#include "../util/FileUtil.h"
#include "../util/HelpFunction.h"
#include <thread>
#include <iostream>
#include <assert.h>
#include <chrono>
#include "ThreadTask.h"
#include "../util/FPSUtil.h"
#include <sys/time.h>
#include <LoadUtil.h>
#include <LightManager.h>
#include <TextureManager.h>
#include <CameraUtil.h>
android_app* MyVulkanManager::Android_application;
bool MyVulkanManager::loopDrawFlag=true;
std::vector<const char *>  MyVulkanManager::instanceExtensionNames;
VkInstance MyVulkanManager::instance;
uint32_t MyVulkanManager::gpuCount;
std::vector<VkPhysicalDevice> MyVulkanManager::gpus;
uint32_t MyVulkanManager::queueFamilyCount;
std::vector<VkQueueFamilyProperties> MyVulkanManager::queueFamilyprops;
uint32_t MyVulkanManager::queueGraphicsFamilyIndex;
VkQueue MyVulkanManager::queueGraphics;
uint32_t MyVulkanManager::queuePresentFamilyIndex;
std::vector<const char *> MyVulkanManager::deviceExtensionNames;
VkDevice MyVulkanManager::device;
VkCommandPool MyVulkanManager::cmdPool;
VkCommandBuffer MyVulkanManager::cmdBuffer;
VkCommandBufferBeginInfo MyVulkanManager::cmd_buf_info;
VkCommandBuffer  MyVulkanManager::cmd_bufs[1];
VkSubmitInfo MyVulkanManager::submit_info[1];
uint32_t MyVulkanManager::screenWidth;
uint32_t MyVulkanManager::screenHeight;
VkSurfaceKHR MyVulkanManager::surface;
std::vector<VkFormat> MyVulkanManager::formats;
VkSurfaceCapabilitiesKHR MyVulkanManager::surfCapabilities;
uint32_t MyVulkanManager::presentModeCount;
std::vector<VkPresentModeKHR> MyVulkanManager::presentModes;
VkExtent2D MyVulkanManager::swapchainExtent;
VkSwapchainKHR MyVulkanManager::swapChain;
uint32_t MyVulkanManager::swapchainImageCount;
std::vector<VkImage> MyVulkanManager::swapchainImages;
std::vector<VkImageView> MyVulkanManager::swapchainImageViews;
VkFormat MyVulkanManager::depthStencilFormat;
VkFormatProperties MyVulkanManager::depthStencilFormatProps;
VkImage MyVulkanManager::depthStencilImage;
VkPhysicalDeviceMemoryProperties MyVulkanManager::memoryroperties;
VkDeviceMemory MyVulkanManager::memDepthStencil;
VkImageView MyVulkanManager::depthStencilImageView;
VkSemaphore MyVulkanManager::imageAcquiredSemaphore;
uint32_t MyVulkanManager::currentBuffer;
VkRenderPass MyVulkanManager::renderPass;
VkClearValue MyVulkanManager::clear_values[2];
VkRenderPassBeginInfo MyVulkanManager::rp_begin;
VkFence MyVulkanManager::taskFinishFence;
VkPresentInfoKHR MyVulkanManager::present;
VkFramebuffer*  MyVulkanManager::framebuffers;
ShaderQueueSuit_Common* MyVulkanManager::sqsCL;
ShaderQueueSuit_NoDepth* MyVulkanManager::sqsND;
ShaderQueueSuit_Mirror* MyVulkanManager::sqsM;
BallForControl*  MyVulkanManager::bfc;
ObjObject*  MyVulkanManager::ballForDraw;
ObjObject*  MyVulkanManager::planeForDraw;
void MyVulkanManager::init_vulkan_instance() {
    AAssetManager* aam=MyVulkanManager::Android_application->activity->assetManager;
    FileUtil::setAAssetManager(aam);
    if (!vk::loadVulkan()){
        LOGI("加载Vulkan图形应用程序接口失败!");
        return ;
    }
    instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instanceExtensionNames.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "HelloVulkan";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "HelloVulkan";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionCount = instanceExtensionNames.size();
    inst_info.ppEnabledExtensionNames = instanceExtensionNames.data();
    inst_info.enabledLayerCount = 0;
    inst_info.ppEnabledLayerNames = NULL;
    VkResult result;
    result = vk::vkCreateInstance(&inst_info, NULL, &instance);
    if(result== VK_SUCCESS){
        LOGE("Vulkan实例创建成功!");
    }
    else{
        LOGE("Vulkan实例创建失败!");
    }
}
void MyVulkanManager::destroy_vulkan_instance() {
    vk::vkDestroyInstance(instance, NULL);
    LOGE("Vulkan实例销毁完毕!");
}
void MyVulkanManager::enumerate_vulkan_phy_devices() {
    gpuCount=0;
    VkResult result = vk::vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);
    assert(result==VK_SUCCESS);
    LOGE("[Vulkan硬件设备数量为%d个]",gpuCount);
    gpus.resize(gpuCount);
    result = vk::vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());
    assert(result==VK_SUCCESS);
    vk::vkGetPhysicalDeviceMemoryProperties(gpus[0],&memoryroperties);
}
void MyVulkanManager::create_vulkan_devices() {
    vk::vkGetPhysicalDeviceQueueFamilyProperties(gpus[0],&queueFamilyCount, NULL);
    LOGE("[Vulkan硬件设备0支持的队列家族数量为%d]",queueFamilyCount);
    queueFamilyprops.resize(queueFamilyCount);
    vk::vkGetPhysicalDeviceQueueFamilyProperties(gpus[0],&queueFamilyCount, queueFamilyprops.data());
    LOGE("[成功获取Vulkan硬件设备0支持的队列家族属性列表]");
    VkDeviceQueueCreateInfo queueInfo = {};
    bool found = false;
    for (unsigned int i = 0; i < queueFamilyCount; i++){
        if (queueFamilyprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            queueInfo.queueFamilyIndex = i;
            queueGraphicsFamilyIndex=i;
            LOGE("[支持GRAPHICS工作的一个队列家族的索引为%d]",i);
            LOGE("[此家族中的实际队列数量是%d]",queueFamilyprops[i].queueCount);
            found = true;
            break;
        }
    }
    float queue_priorities[1] = {0.0};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.pNext = NULL;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = queue_priorities;
    queueInfo.queueFamilyIndex = queueGraphicsFamilyIndex;
    deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = NULL;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = deviceExtensionNames.size();
    deviceInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
    deviceInfo.enabledLayerCount = 0;
    deviceInfo.ppEnabledLayerNames = NULL;
    deviceInfo.pEnabledFeatures = NULL;
    VkResult result = vk::vkCreateDevice(gpus[0], &deviceInfo, NULL, &device);
    assert(result==VK_SUCCESS);
}
void MyVulkanManager::destroy_vulkan_devices() {
    vk::vkDestroyDevice(device, NULL);
    LOGE("逻辑设备销毁完毕！");
}
void MyVulkanManager::create_vulkan_CommandBuffer(){
    VkCommandPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType =VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.pNext = NULL;
    cmd_pool_info.queueFamilyIndex = queueGraphicsFamilyIndex;
    cmd_pool_info.flags =VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult result = vk::vkCreateCommandPool(device,&cmd_pool_info, NULL, &cmdPool);
    assert(result==VK_SUCCESS);
    VkCommandBufferAllocateInfo cmdBAI = {};
    cmdBAI.sType =VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBAI.pNext = NULL;
    cmdBAI.commandPool = cmdPool;
    cmdBAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBAI.commandBufferCount = 1;
    result = vk::vkAllocateCommandBuffers(device,&cmdBAI, &cmdBuffer);
    assert(result==VK_SUCCESS);
    cmd_buf_info.sType =VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buf_info.pNext = NULL;
    cmd_buf_info.flags = 0;
    cmd_buf_info.pInheritanceInfo = NULL;
    cmd_bufs[0] = cmdBuffer;
    VkPipelineStageFlags* pipe_stage_flags = new VkPipelineStageFlags();
    *pipe_stage_flags=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit_info[0].pNext = NULL;
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].pWaitDstStageMask = pipe_stage_flags;
    submit_info[0].commandBufferCount = 1;
    submit_info[0].pCommandBuffers = cmd_bufs;
    submit_info[0].signalSemaphoreCount = 0;
    submit_info[0].pSignalSemaphores = NULL;
}
void MyVulkanManager::destroy_vulkan_CommandBuffer() {
    VkCommandBuffer cmdBufferArray[1] = {cmdBuffer};
    vk::vkFreeCommandBuffers(
            device,
            cmdPool,
            1,
            cmdBufferArray
    );
    vk::vkDestroyCommandPool(device, cmdPool, NULL);
}
void MyVulkanManager::create_vulkan_swapChain() {
    screenWidth = ANativeWindow_getWidth(Android_application->window);
    screenHeight = ANativeWindow_getHeight(Android_application->window);
    LOGE("窗体宽度%d 窗体高度%d",screenWidth,screenHeight);
    VkAndroidSurfaceCreateInfoKHR createInfo;
    createInfo.sType =VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.window = Android_application->window;
    PFN_vkCreateAndroidSurfaceKHR fpCreateAndroidSurfaceKHR=
            (PFN_vkCreateAndroidSurfaceKHR)vk::vkGetInstanceProcAddr(instance,"vkCreateAndroidSurfaceKHR");
    if (fpCreateAndroidSurfaceKHR == NULL){
        LOGE( "找不到vkCreateAndroidSurfaceKHR扩展函数！" );
    }
    VkResult result = fpCreateAndroidSurfaceKHR(instance,&createInfo, nullptr, &surface);
    assert(result==VK_SUCCESS);
    VkBool32 *pSupportsPresent = (VkBool32 *)malloc(queueFamilyCount * sizeof(VkBool32));
    for (uint32_t i = 0; i < queueFamilyCount; i++){
        vk::vkGetPhysicalDeviceSurfaceSupportKHR(gpus[0], i, surface, &pSupportsPresent[i]);
        LOGE("队列家族索引=%d %s显示",i,(pSupportsPresent[i]==1?"支持":"不支持"));
    }
    queueGraphicsFamilyIndex = UINT32_MAX;
    queuePresentFamilyIndex = UINT32_MAX;
    for (uint32_t i = 0; i <queueFamilyCount; ++i)
    {
        if ((queueFamilyprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            if (queueGraphicsFamilyIndex== UINT32_MAX) queueGraphicsFamilyIndex = i;
            if (pSupportsPresent[i] == VK_TRUE)
            {
                queueGraphicsFamilyIndex = i;
                queuePresentFamilyIndex = i;
                LOGE("队列家族索引=%d 同时支持Graphis（图形）和Present（呈现）工作",i);
                break;
            }
        }
    }
    if (queuePresentFamilyIndex == UINT32_MAX)
    {
        for (size_t i = 0; i < queueFamilyCount; ++i)
        {
            if (pSupportsPresent[i] == VK_TRUE)
            {
                queuePresentFamilyIndex= i;
                break;
            }
        }
    }
    free(pSupportsPresent);
    if (queueGraphicsFamilyIndex == UINT32_MAX || queuePresentFamilyIndex == UINT32_MAX)
    {
        LOGE("没有找到支持Graphis（图形）或Present（显示）工作的队列家族");
        assert(false);
    }
    uint32_t formatCount;
    result = vk::vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0],surface, &formatCount, NULL);
    LOGE("支持的格式数量为 %d",formatCount);
    VkSurfaceFormatKHR *surfFormats =(VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    formats.resize(formatCount);
    result = vk::vkGetPhysicalDeviceSurfaceFormatsKHR(gpus[0], surface, &formatCount, surfFormats);
    for(int i=0;i<formatCount;i++){
        formats[i]=surfFormats[i].format;
        LOGE("[%d]支持的格式为%d",i,formats[i]);
    }
    if (formatCount == 1 && surfFormats[0].format== VK_FORMAT_UNDEFINED){
        formats[0] = VK_FORMAT_B8G8R8A8_UNORM;
    }
    free(surfFormats);
    result = vk::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpus[0], surface, &surfCapabilities);
    assert(result == VK_SUCCESS);
    result = vk::vkGetPhysicalDeviceSurfacePresentModesKHR(gpus[0], surface, &presentModeCount, NULL);
    assert(result == VK_SUCCESS);
    LOGE("显示模式数量为%d",presentModeCount);
    presentModes.resize(presentModeCount);
    result = vk::vkGetPhysicalDeviceSurfacePresentModesKHR(gpus[0], surface, &presentModeCount, presentModes.data());
    for(int i=0;i<presentModeCount;i++)
    {
        LOGE("显示模式[%d]编号为%d",i,presentModes[i]);
    }
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (size_t i = 0; i < presentModeCount; i++)
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
        if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR)&&(presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
        {
            swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }
    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
    {
        swapchainExtent.width = screenWidth;
        swapchainExtent.height = screenHeight;
        if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
        {
            swapchainExtent.width = surfCapabilities.minImageExtent.width;
        }
        else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
        {
            swapchainExtent.width = surfCapabilities.maxImageExtent.width;
        }
        if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
        {
            swapchainExtent.height = surfCapabilities.minImageExtent.height;
        } else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
        {
            swapchainExtent.height = surfCapabilities.maxImageExtent.height;
        }
        LOGE("使用自己设置的 宽度 %d 高度 %d",swapchainExtent.width,swapchainExtent.height);
    }
    else
    {
        swapchainExtent = surfCapabilities.currentExtent;
        LOGE("使用获取的surface能力中的 宽度 %d 高度 %d",swapchainExtent.width,swapchainExtent.height);
    }
    screenWidth=swapchainExtent.width;
    screenHeight=swapchainExtent.height;
    uint32_t desiredMinNumberOfSwapChainImages = surfCapabilities.minImageCount+1;
    if ((surfCapabilities.maxImageCount > 0) &&(desiredMinNumberOfSwapChainImages > surfCapabilities.maxImageCount))
    {
        desiredMinNumberOfSwapChainImages = surfCapabilities.maxImageCount;
    }
    VkSurfaceTransformFlagBitsKHR preTransform;
    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        preTransform = surfCapabilities.currentTransform;
    }
    VkSwapchainCreateInfoKHR swapchain_ci = {};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.pNext = NULL;
    swapchain_ci.surface = surface;
    swapchain_ci.minImageCount = desiredMinNumberOfSwapChainImages;
    swapchain_ci.imageFormat = formats[0];
    swapchain_ci.imageExtent.width = swapchainExtent.width;
    swapchain_ci.imageExtent.height = swapchainExtent.height;
    swapchain_ci.preTransform = preTransform;
    swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.presentMode = swapchainPresentMode;
    swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
    swapchain_ci.clipped = true;
    swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_ci.queueFamilyIndexCount = 0;
    swapchain_ci.pQueueFamilyIndices = NULL;
    if (queueGraphicsFamilyIndex != queuePresentFamilyIndex)
    {
        swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_ci.queueFamilyIndexCount = 2;
        uint32_t queueFamilyIndices[2] = {queueGraphicsFamilyIndex,queuePresentFamilyIndex};
        swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
    }
    result = vk::vkCreateSwapchainKHR(device, &swapchain_ci, NULL, &swapChain);
    assert(result == VK_SUCCESS);
    result = vk::vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount, NULL);
    assert(result == VK_SUCCESS);
    LOGE("[SwapChain中的Image数量为%d]",swapchainImageCount);
    swapchainImages.resize(swapchainImageCount);
    result = vk::vkGetSwapchainImagesKHR(device, swapChain, &swapchainImageCount, swapchainImages.data());
    assert(result == VK_SUCCESS);
    swapchainImageViews.resize(swapchainImageCount);
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        VkImageViewCreateInfo color_image_view = {};
        color_image_view.sType =VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_image_view.pNext = NULL;
        color_image_view.flags = 0;
        color_image_view.image = swapchainImages[i];
        color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        color_image_view.format = formats[0];
        color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
        color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
        color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
        color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
        color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_image_view.subresourceRange.baseMipLevel = 0;
        color_image_view.subresourceRange.levelCount = 1;
        color_image_view.subresourceRange.baseArrayLayer = 0;
        color_image_view.subresourceRange.layerCount = 1;
        result = vk::vkCreateImageView(device,&color_image_view, NULL, &swapchainImageViews[i]);
        assert(result == VK_SUCCESS);
    }
}
void MyVulkanManager::destroy_vulkan_swapChain() {
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vk::vkDestroyImageView(device, swapchainImageViews[i], NULL);
        LOGE("[销毁SwapChain ImageView %d 成功]",i);
    }
    vk::vkDestroySwapchainKHR(device, swapChain, NULL);
    LOGE("销毁SwapChain成功!");
}
void MyVulkanManager::create_vulkan_DepthStencilBuffer() { //创建深度、模板缓冲的方法
    depthStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT; //指定深度、模板图像格式
    VkImageCreateInfo image_info = {};
    vk::vkGetPhysicalDeviceFormatProperties(gpus[0], depthStencilFormat, &depthStencilFormatProps);
    if (depthStencilFormatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        image_info.tiling = VK_IMAGE_TILING_LINEAR;
        LOGE("tiling为VK_IMAGE_TILING_LINEAR！");
    }
    else if (depthStencilFormatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        LOGE("tiling为VK_IMAGE_TILING_OPTIMAL！");
    }
    else
    {
        LOGE("不支持VK_FORMAT_D24_UNORM_S8_UINT！");
    }
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = NULL;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = depthStencilFormat;
    image_info.extent.width = screenWidth;
    image_info.extent.height =screenHeight;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_info.queueFamilyIndexCount = 0;
    image_info.pQueueFamilyIndices = NULL;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.flags = 0;
    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = 0;
    mem_alloc.memoryTypeIndex = 0;
    VkImageViewCreateInfo view_info = {}; //构建深度、模板缓冲图像视图创建信息结构体实例
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; //设置结构体类型
    view_info.pNext = NULL; //自定义数据的指针
    view_info.image = VK_NULL_HANDLE; //对应的图像
    view_info.format = depthStencilFormat; //图像视图的格式
    view_info.components.r = VK_COMPONENT_SWIZZLE_R; //设置R 通道调和
    view_info.components.g = VK_COMPONENT_SWIZZLE_G; //设置G 通道调和
    view_info.components.b = VK_COMPONENT_SWIZZLE_B; //设置B 通道调和
    view_info.components.a = VK_COMPONENT_SWIZZLE_A; //设置A 通道调和
    view_info.subresourceRange.aspectMask = //图像视图使用方面
            VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.flags = 0;
    VkResult result = vk::vkCreateImage(device, &image_info, NULL, &depthStencilImage);
    assert(result == VK_SUCCESS);
    VkMemoryRequirements mem_reqs;
    vk::vkGetImageMemoryRequirements(device, depthStencilImage, &mem_reqs);
    mem_alloc.allocationSize = mem_reqs.size;
    VkFlags requirements_mask=0;
    bool flag=memoryTypeFromProperties(memoryroperties, mem_reqs.memoryTypeBits,requirements_mask,&mem_alloc.memoryTypeIndex);
    assert(flag);
    LOGE("确定内存类型成功 类型索引为%d",mem_alloc.memoryTypeIndex);
    result = vk::vkAllocateMemory(device, &mem_alloc, NULL, &memDepthStencil);
    assert(result == VK_SUCCESS);
    result = vk::vkBindImageMemory(device, depthStencilImage, memDepthStencil, 0);
    assert(result == VK_SUCCESS);
    view_info.image = depthStencilImage;
    result = vk::vkCreateImageView(device, &view_info, NULL, &depthStencilImageView);
    assert(result == VK_SUCCESS);
}
void MyVulkanManager::destroy_vulkan_DepthStencilBuffer() {
    vk::vkDestroyImageView(device, depthStencilImageView, NULL);
    vk::vkDestroyImage(device, depthStencilImage, NULL);
    vk::vkFreeMemory(device, memDepthStencil, NULL);
    LOGE("销毁深度缓冲相关成功!");
}
void MyVulkanManager::create_render_pass()  {
    VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
    imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    imageAcquiredSemaphoreCreateInfo.pNext = NULL;
    imageAcquiredSemaphoreCreateInfo.flags = 0;
    VkResult result = vk::vkCreateSemaphore(device,&imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
    assert(result == VK_SUCCESS);
    VkAttachmentDescription attachments[2];
    attachments[0].format = formats[0];
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp=VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout =VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].flags = 0;
    attachments[1].format = depthStencilFormat; //设置深度、模板附件的格式
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT; //设置采样模式
    attachments[1].loadOp = //子通道渲染开始时的操作（针对深度附件）
            VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = //子通道渲染结束时的操作（针对深度附件）
            VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = //子通道渲染开始时的操作（针对模板附件）
            VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = //子通道渲染结束时的操作（针对模板附件）
            VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //开始时的布局
    attachments[1].finalLayout = //结束时的布局
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].flags = 0;
    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout =VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthStencil_reference = {};
    depthStencil_reference.attachment = 1;
    depthStencil_reference.layout =VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthStencil_reference;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext = NULL;
    rp_info.attachmentCount = 2;
    rp_info.pAttachments = attachments;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    rp_info.dependencyCount = 0;
    rp_info.pDependencies = NULL;
    result = vk::vkCreateRenderPass(device, &rp_info, NULL, &renderPass);
    assert(result == VK_SUCCESS);
    clear_values[0].color.float32[0] = 0.0f;
    clear_values[0].color.float32[1] = 0.0f;
    clear_values[0].color.float32[2] = 0.0f;
    clear_values[0].color.float32[3] = 0.0f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;
    rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.pNext = NULL;
    rp_begin.renderPass = renderPass;
    rp_begin.renderArea.offset.x = 0;
    rp_begin.renderArea.offset.y = 0;
    rp_begin.renderArea.extent.width = screenWidth;
    rp_begin.renderArea.extent.height = screenHeight;
    rp_begin.clearValueCount = 2;
    rp_begin.pClearValues = clear_values;
}
void MyVulkanManager::destroy_render_pass() {
    vk::vkDestroyRenderPass(device, renderPass, NULL);
    vk::vkDestroySemaphore(device, imageAcquiredSemaphore, NULL);
}
void MyVulkanManager::init_queue() {
    vk::vkGetDeviceQueue(device, queueGraphicsFamilyIndex, 0,&queueGraphics);
}
void MyVulkanManager::create_frame_buffer() {
    VkImageView attachments[2];
    attachments[1] = depthStencilImageView;
    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.renderPass = renderPass;
    fb_info.attachmentCount = 2;
    fb_info.pAttachments = attachments;
    fb_info.width = screenWidth;
    fb_info.height = screenHeight;
    fb_info.layers = 1;
    uint32_t i;
    framebuffers = (VkFramebuffer *) malloc(swapchainImageCount * sizeof(VkFramebuffer));
    assert(framebuffers);
    for (i = 0; i < swapchainImageCount; i++)
    {
        attachments[0] = swapchainImageViews[i];
        VkResult result = vk::vkCreateFramebuffer(device,&fb_info, NULL, &framebuffers[i]);
        assert(result == VK_SUCCESS);
        LOGE("[创建帧缓冲%d成功！]", i);
    }
}
void MyVulkanManager::destroy_frame_buffer() {
    for (int i = 0; i < swapchainImageCount; i++)
    {
        vk::vkDestroyFramebuffer(device, framebuffers[i], NULL);
    }
    free(framebuffers);
    LOGE("销毁帧缓冲成功！");
}
void MyVulkanManager::createDrawableObject() {
    bfc=new BallForControl(3);
    ballForDraw=LoadUtil::loadFromFile("model/ball.obj",device,memoryroperties);
    planeForDraw=LoadUtil::loadFromFile("model/plane.obj",device,memoryroperties);
}
void MyVulkanManager::destroyDrawableObject() {
    delete ballForDraw;
    delete planeForDraw;
}
void MyVulkanManager::createFence(){
    VkFenceCreateInfo fenceInfo;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = 0;
    vk::vkCreateFence(device, &fenceInfo, NULL, &taskFinishFence);
}
void MyVulkanManager::destroyFence() {
    vk::vkDestroyFence(device, taskFinishFence, NULL);
}
void MyVulkanManager::initPresentInfo() {
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext = NULL;
    present.swapchainCount = 1;
    present.pSwapchains = &swapChain;
    present.waitSemaphoreCount = 0;
    present.pWaitSemaphores = NULL;
    present.pResults = NULL;
}
void MyVulkanManager::initMatrix() {
    MatrixState3D::setInitStack();
    float ratio=(float)screenWidth/(float)screenHeight;
    MatrixState3D::setProjectFrustum(-ratio,ratio,-1,1,3.0f,1000);
    CameraUtil::calCamera(0,0);
    LightManager::setLightPosition(0,2.5f,2.0f);
    LightManager::setlightAmbient(0.15f,0.15f,0.15f,1.0f);
    LightManager::setlightDiffuse(0.9f,0.9f,0.9f,1.0f);
    LightManager::setlightSpecular(0.4f,0.4f,0.4f,1.0f);
}
void MyVulkanManager::flushUniformBuffer() {
    float vertexUniformData[20]=
            {
                    MatrixState3D::cx,MatrixState3D::cy,MatrixState3D::cz,1.0,
                    LightManager::lx,LightManager::ly,LightManager::lz,1.0,
                    LightManager::lightAmbientR,LightManager::lightAmbientG,LightManager::lightAmbientB,LightManager::lightAmbientA,
                    LightManager::lightDiffuseR,LightManager::lightDiffuseG,LightManager::lightDiffuseB,LightManager::lightDiffuseA,
                    LightManager::lightSpecularR,LightManager::lightSpecularG,LightManager::lightSpecularB,LightManager::lightSpecularA
            };
    uint8_t *pData;
    VkResult result = vk::vkMapMemory(device, sqsCL->memUniformBuf,0, sqsCL->bufferByteCount, 0, (void **)&pData);
    assert(result==VK_SUCCESS);
    memcpy(pData, vertexUniformData, sqsCL->bufferByteCount);
    vk::vkUnmapMemory(device,sqsCL->memUniformBuf);
    uint8_t *pDataN;
    result = vk::vkMapMemory(device, sqsND->memUniformBuf,0, sqsND->bufferByteCount, 0, (void **)&pDataN);
    assert(result==VK_SUCCESS);
    memcpy(pDataN, vertexUniformData, sqsND->bufferByteCount);
    vk::vkUnmapMemory(device,sqsND->memUniformBuf);
    uint8_t *pDataM;
    result = vk::vkMapMemory(device, sqsM->memUniformBuf,0, sqsM->bufferByteCount, 0, (void **)&pDataM);
    assert(result==VK_SUCCESS);
    memcpy(pDataM, vertexUniformData, sqsM->bufferByteCount);
    vk::vkUnmapMemory(device,sqsM->memUniformBuf);
}
void MyVulkanManager::flushTexToDesSet() {
    for(int i=0;i<TextureManager::texNames.size();i++)
    {
        sqsCL->writes[0].dstSet = sqsCL->descSet[i];
        sqsCL->writes[1].dstSet = sqsCL->descSet[i];
        sqsCL->writes[1].pImageInfo = &(TextureManager::texImageInfoList[TextureManager::texNames[i]]);
        vk::vkUpdateDescriptorSets(device, 2, sqsCL->writes, 0, NULL);
    }
    for(int i=0;i<TextureManager::texNames.size();i++)
    {
        sqsND->writes[0].dstSet = sqsND->descSet[i];
        sqsND->writes[1].dstSet = sqsND->descSet[i];
        sqsND->writes[1].pImageInfo = &(TextureManager::texImageInfoList[TextureManager::texNames[i]]);
        vk::vkUpdateDescriptorSets(device, 2, sqsND->writes, 0, NULL);
    }

    for(int i=0;i<TextureManager::texNames.size();i++)
    {
        sqsM->writes[0].dstSet = sqsM->descSet[i];
        sqsM->writes[1].dstSet = sqsM->descSet[i];
        sqsM->writes[1].pImageInfo = &(TextureManager::texImageInfoList[TextureManager::texNames[i]]);
        vk::vkUpdateDescriptorSets(device, 2, sqsM->writes, 0, NULL);
    }
}
void MyVulkanManager::drawObject() {
    FPSUtil::init();
    while(MyVulkanManager::loopDrawFlag){
        FPSUtil::calFPS();
        FPSUtil::before();
        LightManager::move();
        bfc->step();
        CameraUtil::flushCameraToMatrix();
        VkResult result = vk::vkAcquireNextImageKHR(device, swapChain,UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,&currentBuffer);
        rp_begin.framebuffer = framebuffers[currentBuffer];
        vk::vkResetCommandBuffer(cmdBuffer, 0);
        result = vk::vkBeginCommandBuffer(cmdBuffer, &cmd_buf_info);
        MyVulkanManager::flushUniformBuffer();
        MyVulkanManager::flushTexToDesSet();
        vk::vkCmdBeginRenderPass(cmdBuffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
        MatrixState3D::pushMatrix(); //保护现场
        MatrixState3D::scale(0.7,1.0,0.5); //执行缩放变换
        planeForDraw->drawSelf(cmdBuffer,sqsCL->pipelineLayout,sqsCL->pipeline, //绘制木地板
                               &(sqsCL->descSet[TextureManager::getVkDescriptorSetIndex("texture/mdb.bntex")]),false);
        MatrixState3D::popMatrix(); //恢复现场
        MatrixState3D::pushMatrix(); //保护现场
        MatrixState3D::scale(1,-1,1);
        MatrixState3D::translate(0,bfc->currentY+0.4,0); //执行平移变换（镜像球位置）
        ballForDraw->drawSelf(cmdBuffer,sqsND->pipelineLayout,sqsND->pipeline, //绘制镜像球
                              &(sqsND->descSet[TextureManager::getVkDescriptorSetIndex("texture/ball.bntex")]),true);
        MatrixState3D::popMatrix(); //恢复现场
        MatrixState3D::pushMatrix();
        MatrixState3D::scale(0.7,1.0,0.5);
        planeForDraw->drawSelf(cmdBuffer,sqsM->pipelineLayout,sqsM->pipeline,
                               &(sqsM->descSet[TextureManager::getVkDescriptorSetIndex("texture/mdbtm.bntex")]),false);
        MatrixState3D::popMatrix();
        MatrixState3D::pushMatrix();
        MatrixState3D::translate(0,bfc->currentY+0.4,0);
        ballForDraw->drawSelf(cmdBuffer,sqsCL->pipelineLayout,sqsCL->pipeline,
                              &(sqsCL->descSet[TextureManager::getVkDescriptorSetIndex("texture/ball.bntex")]),false);
        MatrixState3D::popMatrix();
        vk::vkCmdEndRenderPass(cmdBuffer);
        result = vk::vkEndCommandBuffer(cmdBuffer);
        submit_info[0].waitSemaphoreCount = 1;
        submit_info[0].pWaitSemaphores =  &imageAcquiredSemaphore;
        result = vk::vkQueueSubmit(queueGraphics, 1, submit_info, taskFinishFence);
        do{
            result = vk::vkWaitForFences(device, 1, &taskFinishFence, VK_TRUE, FENCE_TIMEOUT);
        }
        while (result == VK_TIMEOUT);
        vk::vkResetFences(device,1,&taskFinishFence);
        present.pImageIndices = &currentBuffer;
        result = vk::vkQueuePresentKHR(queueGraphics, &present);
        FPSUtil::after(60);
    }
}
void MyVulkanManager::doVulkan() {
    ThreadTask* tt=new ThreadTask();
    thread t1(&ThreadTask::doTask,tt);
    t1.detach();
}
void MyVulkanManager::init_texture() {
    TextureManager::initTextures(device,gpus[0],memoryroperties,cmdBuffer,queueGraphics);
}
void MyVulkanManager::destroy_textures() {
    TextureManager::deatroyTextures(device);
}
void MyVulkanManager::initPipeline() {
    sqsCL=new ShaderQueueSuit_Common(&device,renderPass,memoryroperties);
    sqsND=new ShaderQueueSuit_NoDepth(&device,renderPass,memoryroperties);
    sqsM=new ShaderQueueSuit_Mirror(&device,renderPass,memoryroperties);
}
void MyVulkanManager::destroyPipeline() {
    delete sqsCL;
    delete sqsND;
    delete sqsM;
}