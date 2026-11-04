#ifndef ZERO_NET_UTIL_H
#define ZERO_NET_UTIL_H

#include <string>

namespace zero {

// GET a URL and return the body. Returns false (and fills error) on failure.
bool HttpGetText(const std::string& url, std::string& body, std::string& error);

std::string UrlEncode(const std::string& value);

}  // namespace zero

#endif  // ZERO_NET_UTIL_H
