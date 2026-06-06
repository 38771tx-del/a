#include "http.hpp"
#include "../curl/curl.h"

namespace http {
struct slot {
	request request;
	CURL* easy = nullptr;
	curl_slist* headers = nullptr;
	std::string response;
};

static size_t write_cb(void* ptr, size_t size, size_t nmemb, void* userdata) {
	size_t realsize = size * nmemb;
	static_cast<std::string*>(userdata)->append(static_cast<char*>(ptr), realsize);
	return realsize;
}

std::vector<result> curl_multi(const std::vector<request>& requests) {
	std::vector<result> out(requests.size());
	if (requests.empty()) return out;
	CURLM* multi = curl_multi_init();
	if (!multi) return out;
	std::vector<slot> slots(requests.size());
	for (std::size_t i = 0; i < requests.size(); ++i) slots[i].request = requests[i];

	int running = 0;
	auto add_request = [&](std::size_t i) {
		slot& current = slots[i];
		current.easy = curl_easy_init();
		if (!current.easy) return;
		current.response.clear();
		curl_easy_setopt(current.easy, CURLOPT_URL, current.request.url.c_str());
		curl_easy_setopt(current.easy, CURLOPT_WRITEFUNCTION, write_cb);
		curl_easy_setopt(current.easy, CURLOPT_WRITEDATA, &current.response);
		curl_easy_setopt(current.easy, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(current.easy, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(current.easy, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(current.easy, CURLOPT_SSL_VERIFYHOST, 0L);
		if (!current.request.cookie.empty())
			curl_easy_setopt(current.easy, CURLOPT_COOKIE, current.request.cookie.c_str());
		if (current.request.post) {
			current.headers = curl_slist_append(current.headers, "Content-Type: application/json");
			curl_easy_setopt(current.easy, CURLOPT_POST, 1L);
			curl_easy_setopt(current.easy, CURLOPT_POSTFIELDS, current.request.body.c_str());
			curl_easy_setopt(current.easy, CURLOPT_HTTPHEADER, current.headers);
		}
		curl_easy_setopt(current.easy, CURLOPT_PRIVATE, &current);
		curl_multi_add_handle(multi, current.easy);
		running++;
	};

	for (std::size_t i = 0; i < requests.size(); ++i) add_request(i);

	while (running > 0) {
		curl_multi_perform(multi, &running);

		int msgs = 0;
		while (CURLMsg* msg = curl_multi_info_read(multi, &msgs)) {
			if (msg->msg != CURLMSG_DONE) continue;

			CURL* easy = msg->easy_handle;
			slot* current = nullptr;
			curl_easy_getinfo(easy, CURLINFO_PRIVATE, &current);

			if (current) {
				std::size_t idx = static_cast<std::size_t>(current - slots.data());
				long status = 0;
				curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &status);
				out[idx].status = status;
				out[idx].body = std::move(current->response);
				out[idx].ok = (status == 200);
			}

			curl_multi_remove_handle(multi, easy);
			curl_easy_cleanup(easy);
			if (current) current->easy = nullptr;
		}

		if (running > 0) curl_multi_poll(multi, nullptr, 0, 440, nullptr);
	}

	for (std::size_t i = 0; i < slots.size(); ++i) {
		if (slots[i].easy) {
			curl_multi_remove_handle(multi, slots[i].easy);
			curl_easy_cleanup(slots[i].easy);
			slots[i].easy = nullptr;
		}
		if (slots[i].headers) {
			curl_slist_free_all(slots[i].headers);
			slots[i].headers = nullptr;
		}
	}

	curl_multi_cleanup(multi);
	return out;
}
}
