#include "agent_guide.h"
#include <string.h>
#include <strings.h>

namespace {
	// Placeholder the device address replaces
	constexpr const char *HOST_PLACEHOLDER = "{host}";

	// The guide, with the address as a placeholder (kept in flash)
	constexpr const char *AGENT_GUIDE = R"(# Gizmo Ticker

A crypto price display on the local network: an ESP32-S3 that scrolls the market data of one coin on a MAX7219 LED matrix. The coin and what the matrix shows are set by its owner in its web app, and the market data comes from CoinGecko.

## HTTP API

The ticker answers plain HTTP requests on the local network:

- `GET http://{host}/api/config` returns the settings of the ticker as JSON: the coin, the sections the matrix shows, the number format, the brightness, the scroll speed and the size of the matrix.

```sh
curl "http://{host}/api/config"
```

## Tips

- `cryptoCoin` is the CoinGecko ID of the coin (for example `bitcoin` or `ethereum`), not its symbol.
- The sections the matrix scrolls through are `currentPrice`, `priceChange` (over 24 hours), `marketCap`, `dailyHighLow`, `yearHighLow`, `openPrice` and `volume`: each is `true` when it is shown. Prices are in US dollars.
- `formatType` is `US` (1,234.56) or `EU` (1.234,56); `matrixModules` counts the 8x8 blocks of the matrix, in steps of 4.
- The ticker shows its prices on the matrix only: read them from CoinGecko for the same coin.
- `apiKey` is masked, and the ticker never sends its API key back.
- Errors of the API are JSON: `{ "errorId": "...", "errorMessage": "...", "data": null }`.
- The ticker only answers on its local network, and without authentication: anyone on that network can read it.
)";

	// Find the position of a media type in an Accept header, ignoring the case (-1 if it isn't there)
	int findMediaType(const char *accept, const char *mediaType) {
		const size_t acceptLength = strlen(accept);
		const size_t mediaTypeLength = strlen(mediaType);
		for (size_t position = 0; position + mediaTypeLength <= acceptLength; position++) {
			if (strncasecmp(accept + position, mediaType, mediaTypeLength) == 0) {
				return static_cast<int>(position);
			}
		}
		return -1;
	}
}

// Write the guide piece by piece, with the address the agent used
void writeAgentGuide(const char *host, AgentGuideWriter write, void *context) {
	const size_t placeholderLength = strlen(HOST_PLACEHOLDER);
	const char *remaining = AGENT_GUIDE;

	// Write the text between placeholders, and the address in their place
	const char *placeholder = strstr(remaining, HOST_PLACEHOLDER);
	while (placeholder != nullptr) {
		write(remaining, static_cast<size_t>(placeholder - remaining), context);
		write(host, strlen(host), context);
		remaining = placeholder + placeholderLength;
		placeholder = strstr(remaining, HOST_PLACEHOLDER);
	}

	// Write the end of the guide
	write(remaining, strlen(remaining), context);
}

// Check whether a client prefers Markdown to HTML from its Accept header
bool isMarkdownPreferred(const char *accept) {
	// No header: a plain request, which gets the web app
	if (accept == nullptr) {
		return false;
	}

	// Markdown must be listed, and come before HTML when both are (the quality values are not weighed)
	const int markdownPosition = findMediaType(accept, "text/markdown");
	const int htmlPosition = findMediaType(accept, "text/html");
	return markdownPosition >= 0 && (htmlPosition < 0 || markdownPosition < htmlPosition);
}