#include <gizmo/wifi_setup.h>
#include "server.h"
#include "../config/config.h"
#include "../webui/generated_assets.h"

namespace {
	constexpr const char* NOT_FOUND_ROUTE = "/404.html";

	// Normalize the request path by removing trailing slashes
	String normalizeAssetPath(const String& requestPath) {
		if (requestPath.length() <= 1 || !requestPath.endsWith("/")) return requestPath;
		return requestPath.substring(0, requestPath.length() - 1);
	}

	// Find the embedded asset that matches the request path
	const EmbeddedAsset* findEmbeddedAsset(const String& requestPath) {
		const String normalizedPath = normalizeAssetPath(requestPath);

		// Loop all embedded assets
		for (size_t i = 0; i < EMBEDDED_ASSET_COUNT; i++) {
			if (normalizedPath.equals(EMBEDDED_ASSETS[i].routePath)) {
				return &EMBEDDED_ASSETS[i];
			}
		}

		// No matching asset found
		return NULL;
	}

	// Send the embedded asset
	bool sendEmbeddedAsset(AsyncWebServerRequest *request, const String& requestPath, uint16_t statusCode = 200) {
		// Find the asset for the requested path
		const EmbeddedAsset* asset = findEmbeddedAsset(requestPath);
		if (asset == NULL) {
			return false;
		}

		// Send the asset data as the response
		AsyncWebServerResponse *response = request->beginResponse(statusCode, asset->contentType, asset->data, asset->size);
		if (asset->gzip) {
			response->addHeader("Content-Encoding", "gzip");
		}

		// Send the response
		request->send(response);
		return true;
	}

	// Detect whether the request expects an HTML document instead of a static asset or API payload
	bool shouldServeNotFoundPage(AsyncWebServerRequest *request) {
		const String requestPath = request->url();

		// Don't serve the 404 page for API or asset requests
		if (requestPath.startsWith("/api/") || requestPath.startsWith("/_nuxt/")) {
			return false;
		}

		// If the request accepts HTML or doesn't have a file extension, serve the 404 page
		if (request->hasHeader("Accept") && request->getHeader("Accept")->value().indexOf("text/html") >= 0) {
			return true;
		}

		// Serve the 404 page for requests without a file extension
		const int lastSlashIndex = requestPath.lastIndexOf('/');
		const int lastDotIndex = requestPath.lastIndexOf('.');
		return lastDotIndex <= lastSlashIndex;
	}
}

// Setup the captive portal route to handle all unmatched requests
void setupCaptivePortalRoute() {
	server.onNotFound([](AsyncWebServerRequest *request) {
		// Redirect hotspot clients to the Wi-Fi setup page
		if (gizmo::handleCaptivePortalRequest(request)) {
			return;
		}

		// Serve embedded assets
		if ((request->method() == HTTP_GET || request->method() == HTTP_HEAD) && sendEmbeddedAsset(request, request->url())) {
			return;
		}

		// Serve the prerendered the 404 page for missing document requests
		if ((request->method() == HTTP_GET || request->method() == HTTP_HEAD) && shouldServeNotFoundPage(request) && sendEmbeddedAsset(request, NOT_FOUND_ROUTE, 404)) {
			return;
		}

		// If no route matched, return 404
		request->send(404);
	});
}