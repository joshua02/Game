#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>



#ifdef NDEBUG
const bool enableValidationLayers = false;
const std::vector<const char*> desiredInstanceLayers = {};
#else
const bool enableValidationLayers = true;
const std::vector<const char*> desiredInstanceLayers = {
	"VK_LAYER_KHRONOS_validation"
};
#endif



//is this needed?
std::vector<const char*> desiredInstanceExtensions = {};

const std::vector<const char*> desiredDeviceExtensions = { "VK_KHR_swapchain" };



void throwError(std::string error) {
	throw std::runtime_error(error);
}


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

class Renderer {
public:
	void run() {
		initWindow();

		initVulkan();
		setupDebugMessenger();

		getPhysicalDevice();
		getQueues();
		createLogicalDevice();

		mainLoop();
		cleanup();
	}
private:
	SDL_Window* window;
	bool running = true;


	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice;
	VkDevice device;

	uint32_t graphicsQueueIndex;
	uint32_t presentQueueIndex;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	void initWindow() {
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			throwError("Failed to init SDL D:");
		}

		window = SDL_CreateWindow(
			"Game of the Century",
			640,
			480,
			SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
		);

		if (window == NULL) {
			throwError("Failed to init window D:");
		}
	}

	void mainLoop() {
		while (running == true) {
			SDL_Event event;

			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_QUIT) {
					running = false;
				}
			}
		}
	}

	void cleanup() {
		
		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroyInstance(instance, nullptr);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void createLogicalDevice() {
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = NULL;
		createInfo.queueCreateInfoCount = 2;


		float queuePriority = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfos[2];
		queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[0].pNext = nullptr;
		queueCreateInfos[0].flags = 0;
		queueCreateInfos[0].queueFamilyIndex = graphicsQueueIndex;
		queueCreateInfos[0].queueCount = 1;
		queueCreateInfos[0].pQueuePriorities = &queuePriority;

		queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[1].pNext = nullptr;
		queueCreateInfos[1].flags = 0;
		queueCreateInfos[1].queueFamilyIndex = presentQueueIndex;
		queueCreateInfos[1].queueCount = 1;
		queueCreateInfos[1].pQueuePriorities = &queuePriority;


		createInfo.pQueueCreateInfos = queueCreateInfos;

		if (checkDeviceExtensions() == false) {
			throwError("Device extensions not supported D:");
		}
		
		createInfo.enabledExtensionCount = desiredDeviceExtensions.size();
		createInfo.ppEnabledExtensionNames = desiredDeviceExtensions.data();



		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throwError("Failed to create logical device D:");
		}

		vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
		vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);

	}

	//prioritizes a queue supporting both graphics and transfer/present
	//TODO: add compute queue ?
	void getQueues() {

		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilies.data());

		/*for (int i = 0; i < count; i++) {
			if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0) {
				graphicsQueue = i;
				presentQueue = i;
				return;
			}
		}*/
		bool foundGraphics = false;
		bool foundPresent = false;
		for (int i = 0; i < count; i++) {
			if (foundGraphics == false && ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)) {
				graphicsQueueIndex = i;
				foundGraphics = true;
				if (foundPresent) return;
				continue;
			}
			if (foundPresent == false && ((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)) {
				presentQueueIndex = i;
				foundPresent = true;
				if (foundGraphics) return;
			}
		}

		throwError("Failed to find queues D:");

	}

	//set the physical device, prioritizes discrete gpu
	void getPhysicalDevice() {
		uint32_t count;
		vkEnumeratePhysicalDevices(instance, &count, nullptr);
		
		std::vector<VkPhysicalDevice> physicalDevices(count);
		vkEnumeratePhysicalDevices(instance, &count, physicalDevices.data());

		VkPhysicalDeviceProperties props{};
		for (auto device : physicalDevices) {
			vkGetPhysicalDeviceProperties(device, &props);

			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				physicalDevice = device;
				return;
			}

			/*std::cout << props.deviceName << std::endl;
			std::cout << props.deviceType << std::endl;*/
		}

		for (auto device : physicalDevices) {
			vkGetPhysicalDeviceProperties(device, &props);

			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
				physicalDevice = device;
				return;
			}

			/*std::cout << props.deviceName << std::endl;
			std::cout << props.deviceType << std::endl;*/
		}

		throwError("No gpu found");

	}

	void initVulkan() {

		if (checkLayers() == false) {
			throwError("Not all layers available D:");
		}
		

		uint32_t count;
		const char* const* sdlInstances = SDL_Vulkan_GetInstanceExtensions(&count);

		for (int i = 0; i < count; i++) {
			desiredInstanceExtensions.push_back(sdlInstances[i]);
		}

		if (enableValidationLayers) {
			desiredInstanceExtensions.push_back("VK_EXT_debug_utils");
		}

		if (checkExtensions() == false) {
			throwError("Not all extensions available D:");
		}

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.flags = NULL;
		createInfo.pApplicationInfo = nullptr;
		createInfo.enabledLayerCount = desiredInstanceLayers.size();
		createInfo.ppEnabledLayerNames = desiredInstanceLayers.data();
		createInfo.enabledExtensionCount = desiredInstanceExtensions.size();
		createInfo.ppEnabledExtensionNames = desiredInstanceExtensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
		}
		else {
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throwError("Failed to create instance D:");
		}
	}

	bool checkDeviceExtensions() {
		//logical extensions
		uint32_t count;
		vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &count, nullptr);
		std::vector<VkExtensionProperties> availableLogicalExtensions(count);
		vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &count, availableLogicalExtensions.data());

		//for (const auto& ext : availableLogicalExtensions) {
		//	std::cout << ext.extensionName << std::endl;
		//}

		bool hasExt = false;
		for (auto desired : desiredDeviceExtensions) {
			for (const auto& ext : availableLogicalExtensions) {
				if (std::strcmp(ext.extensionName, desired)) {
					hasExt = true;
					break;
				}
			}
			if (hasExt == false) {
				return false;
			}
		}
		return true;
	}

	bool checkLayers() {
		std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

		//for (auto layer : availableLayers) {
		//	std::cout << layer.layerName << std::endl;
		//}

		bool hasLayer = false;
		for (auto desired : desiredInstanceLayers) {
			for (auto layer : availableLayers) {
				if (std::strcmp(layer.layerName, desired)) {
					hasLayer = true;
					break;
				}
			}
			if (hasLayer == false) {
				return false;
			}
		}
		return true;
	}

	bool checkExtensions() {
		std::vector<vk::ExtensionProperties> availableExtensions = vk::enumerateInstanceExtensionProperties();

		//for (auto extension : availableExtensions) {
		//	std::cout << extension.extensionName << std::endl;
		//}

		bool hasExtension = false;
		for (auto desired : desiredInstanceExtensions) {
			for (auto layer : availableExtensions) {
				if (std::strcmp(layer.extensionName, desired)) {
					hasExtension = true;
					break;
				}
			}
			if (hasExtension == false) {
				return false;
			}
		}
		return true;
	}

	//from https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}


	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throwError("failed to set up debug messenger D:");
		}
	}

};

int main() {

	std::cout << "Hello Gamers" << std::endl;

	Renderer renderer;

	try {
		renderer.run();
	}
	catch (std::exception e) {
		std::cerr << e.what();
	}


	return 0;
}