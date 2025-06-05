#include <iostream>

#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>


//TODO: add flag to turn off validation layer for release build
const std::vector<const char*> desiredInstanceLayers = {
	"VK_LAYER_KHRONOS_validation"
};

//is this needed?
const std::vector<const char*> desiredInstanceExtensions = {
	"VK_KHR_surface"
};



void throwError(std::string error) {
	throw std::runtime_error(error);
}




class Renderer {
public:
	void run() {
		initWindow();

		initvulkan();

		mainLoop();
		cleanup();
	}
private:
	SDL_Window* window;
	bool running = true;


	VkInstance instance;


	void initWindow() {
		if (!SDL_Init(SDL_INIT_VIDEO)) {
			throwError("Failed to init SDL D:");
		}

		window = SDL_CreateWindow(
			"Game of the Century",                  // window title
			640,                               // width, in pixels
			480,                               // height, in pixels
			SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE                  // flags - see below
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

		vkDestroyInstance(instance, nullptr);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void initvulkan() {

		if (checkLayers() == false) {
			throwError("Not all layers available D:");
		}
		if (checkExtensions() == false) {
			throwError("Not all layers available D:");
		}

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.flags = NULL;
		createInfo.pApplicationInfo = NULL;
		createInfo.enabledLayerCount = desiredInstanceLayers.size();
		createInfo.ppEnabledLayerNames = desiredInstanceLayers.data();
		createInfo.enabledExtensionCount = desiredInstanceExtensions.size();
		createInfo.ppEnabledExtensionNames = desiredInstanceExtensions.data();

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throwError("Failed to create instance D:");
		}

		/*
		* TODO:
		an application can link a VkDebugReportCallbackCreateInfoEXT structure
		or a VkDebugUtilsMessengerCreateInfoEXT structure to the pNext chain of
		the VkInstanceCreateInfo structure passed to vkCreateInstance.
		*/
	}

	bool checkLayers() {
		std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

		/*for (auto layer : availableLayers) {
			std::cout << layer.layerName << std::endl;
		}*/

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