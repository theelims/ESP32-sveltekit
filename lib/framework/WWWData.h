#include <functional>
#include <Arduino.h>
typedef std::function<void(const String& uri, const String& contentType, const uint8_t * content, size_t len)> RouteRegistrationHandler;

class WWWData {
	public:
		static void registerRoutes(RouteRegistrationHandler handler) {
		}
};

