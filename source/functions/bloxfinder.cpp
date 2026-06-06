#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "bloxfinder.hpp"
#include "../cache/sdk.hpp"
#include "../json/simdjson.h"
#include "../utils/flat_hash_map.hpp"
#include "../utils/http.hpp"
#include "../utils/memory.hpp"
#include "../utils/offsets.hpp"

int bloxfinder_run(const char* user_id, std::string& luminant_name, std::string& server_name) {
	luminant_name.clear();
	server_name.clear();
	Instance servers = SDK::DataModel.FindFirstChildOfClass("ReplicatedStorage").FindFirstChild("Servers");
	if (!servers.address) return 2;

	ska::flat_hash_map<std::string, std::pair<std::string, std::string>> job_map;
	job_map.reserve(512);
	for (const char* luminant : { "Depths", "EastLuminant", "EtreanLuminant" }) {
		Instance group = servers.FindFirstChild(luminant);
		std::vector<std::uint64_t> jobs;
		group.GetChildren(jobs);
		for (std::uint64_t address : jobs) {
			Instance job{ address };
			Instance rich = job.FindFirstChild("RichName");
			if (!rich.address) continue;
			std::string value = mem::read_string(rich.address + Offsets::Misc::Value);
			std::string id = job.GetInstanceName();
			job_map[id] = { luminant, value };
		}
	}
	if (job_map.empty()) return 1;

	constexpr const char* place_ids[] = { "6473861193", "5735553160", "6032399813" };
	static const std::string cookie = ".ROBLOSECURITY=_|WARNING:-DO-NOT-SHARE-THIS.--Sharing-this-will-allow-someone-to-log-in-as-you-and-to-steal-your-ROBUX-and-items.|"; // removed cookie for security purposes :(

	std::vector<http::request> requests;
	requests.push_back({ std::string("https://thumbnails.roblox.com/v1/users/avatar-headshot?userIds=") + user_id + "&size=150x150&format=Png&isCircular=false", "", cookie, false });
	for (const char* place_id : place_ids) {
		requests.push_back({ std::string("https://games.roblox.com/v1/games/") + place_id + "/servers/Public?limit=100", "", cookie, false });
	}

	std::vector<http::result> results = http::curl_multi(requests);
	if (results.size() < 4 || !results[0].ok) return 1;

	simdjson::ondemand::parser parser;
	auto parse = [&](std::string& body, std::string& next_cursor, std::vector<std::pair<std::string, std::string>>& out) {
		next_cursor.clear();
		simdjson::padded_string_view padded = simdjson::pad(body);
		simdjson::ondemand::document doc;
		(void)parser.iterate(padded).get(doc);
		simdjson::ondemand::array data;
		(void)doc["data"].get_array().get(data);
		for (auto server : data) {
			std::string_view id{};
			(void)server["id"].get_string().get(id);
			simdjson::ondemand::array tokens;
			(void)server["playerTokens"].get_array().get(tokens);
			for (auto token : tokens) {
				std::string_view value{};
				(void)token.get_string().get(value);
				out.emplace_back(std::string(value), std::string(id));
			}
		}
		std::string_view cursor{};
		(void)doc["nextPageCursor"].get_string().get(cursor);
		next_cursor.assign(cursor.data(), cursor.size());
	};

	std::string avatar_url;
	{
		simdjson::padded_string_view padded = simdjson::pad(results[0].body);
		simdjson::ondemand::document doc;
		(void)parser.iterate(padded).get(doc);
		simdjson::ondemand::array data;
		(void)doc["data"].get_array().get(data);
		for (auto item : data) {
			std::string_view img{};
			(void)item["imageUrl"].get_string().get(img);
			avatar_url.assign(img.data(), img.size());
			break;
		}
	}
	if (avatar_url.find("tr.rbxcdn.com") == std::string::npos) return 1;

	std::vector<std::pair<std::string, std::string>> token_jobs;
	std::vector<std::string> cursors(3);
	for (std::size_t i = 0; i < 3; ++i) {
		if (!results[i + 1].ok) continue;
		parse(results[i + 1].body, cursors[i], token_jobs);
	}

	std::vector<http::request> next_requests;
	std::vector<std::size_t> next_indexes;
	for (;;) {
		next_requests.clear();
		next_indexes.clear();
		for (std::size_t i = 0; i < 3; ++i) {
			if (cursors[i].empty()) continue;
			next_requests.push_back({ std::string("https://games.roblox.com/v1/games/") + place_ids[i] + "/servers/Public?limit=100&cursor=" + cursors[i], "", cookie, false });
			next_indexes.push_back(i);
		}
		if (next_requests.empty()) break;

		std::vector<http::result> next_results = http::curl_multi(next_requests);
		for (std::size_t i = 0; i < next_indexes.size(); ++i) {
			if (i >= next_results.size() || !next_results[i].ok) continue;
			parse(next_results[i].body, cursors[next_indexes[i]], token_jobs);
		}
	}

	if (token_jobs.empty()) return 1;

	std::vector<http::request> batch_requests;
	for (std::size_t i = 0; i < token_jobs.size(); i += 100) {
		std::size_t end = (i + 100 < token_jobs.size()) ? (i + 100) : token_jobs.size();
		std::string body;
		body.reserve(((end - i) * 256) + 2);
		body.push_back('[');
		for (std::size_t k = i; k < end; ++k) {
			if (k != i) body.push_back(',');
			body += "{\"token\":\"";
			body += token_jobs[k].first;
			body += "\",\"type\":\"AvatarHeadshot\",\"size\":\"150x150\",\"requestId\":\"";
			body += token_jobs[k].second;
			body += "\"}";
		}
		body.push_back(']');
		batch_requests.push_back({ "https://thumbnails.roblox.com/v1/batch", std::move(body), cookie, true });
	}

	std::vector<http::result> batch_results = http::curl_multi(batch_requests);
	for (http::result& result : batch_results) {
		if (!result.ok) continue;
		simdjson::padded_string_view padded = simdjson::pad(result.body);
		simdjson::ondemand::document doc;
		(void)parser.iterate(padded).get(doc);
		simdjson::ondemand::array data;
		(void)doc["data"].get_array().get(data);
		for (auto item : data) {
			std::string_view image{};
			std::string_view request{};
			(void)item["imageUrl"].get_string().get(image);
			(void)item["requestId"].get_string().get(request);
			if (image != avatar_url) continue;
			auto it = job_map.find(std::string(request));
			if (it == job_map.end()) continue;
			std::string out;
			for (std::size_t i = 0; i < it->second.first.size(); ++i) {
				if (i > 0 && it->second.first[i] >= 'A' && it->second.first[i] <= 'Z' && it->second.first[i - 1] != 32)
					out += 32;
				out += it->second.first[i];
			}
			luminant_name = std::move(out);
			server_name = it->second.second;
			return 0;
		}
	}

	return 1;
}
