#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace http {
struct result {
	long status = 0;
	std::string body;
	bool ok = false;
};

struct request {
	std::string url;
	std::string body;
	std::string cookie;
	bool post = false;
};

std::vector<result> curl_multi(const std::vector<request>& requests);
}
