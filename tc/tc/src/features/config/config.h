#pragma once
#include <string>
#include <vector>

namespace config
{
	struct config_info_t
	{
		std::string name;
		std::string path;
	};

	std::string get_config_directory();
	std::string get_config_path(const std::string& name);
	bool ensure_config_directory();
	
	std::vector<config_info_t> get_config_list();
	bool save_config(const std::string& name);
	bool load_config(const std::string& name);
	bool delete_config(const std::string& name);
	bool config_exists(const std::string& name);
	void open_file_location();
}
