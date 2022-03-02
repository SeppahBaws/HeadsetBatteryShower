#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std::literals;
using namespace std::string_literals;
namespace fs = std::filesystem;

std::string GetExePath()
{
	char result[MAX_PATH]{};
	const std::string path = std::string(result, GetModuleFileNameA(nullptr, result, MAX_PATH));

	const size_t pos = path.find_last_of("/\\");
	return path.substr(0, pos + 1);
}

int main()
{
	try
	{
		std::string iCueDir;

		// Read iCue directory from config file.
		{
			const std::string configPath = GetExePath() + "iCueDir.txt";
			std::ifstream file(configPath, std::ios::binary);
			if (!file)
			{
				throw std::runtime_error("Unable to open config file: '"s + configPath + "'"s);
			}

			file.seekg(0, std::ios::beg);
			std::getline(file, iCueDir);
			file.close();

			if (iCueDir.empty())
			{
				throw std::runtime_error("iCue directory could not be read!");
			}
		}

		std::vector<fs::directory_entry> files;

		// Get all .log files in directory.
		for (const auto& entry : fs::directory_iterator(iCueDir))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".log")
			{
				files.push_back(entry);
			}
		}

		if (files.empty())
		{
			throw std::runtime_error("No .log files present in logs folder");
		}

		// Sort files based on last write time.
		std::ranges::sort(files, [](const fs::directory_entry& left, const fs::directory_entry& right)
			{
				return left.last_write_time() > right.last_write_time();
			});

		const fs::path path = files[0].path();
		std::ifstream file(path);
		if (!file.is_open())
			throw std::runtime_error(("Failed to open file \""s + path.string() + "\"").c_str());

		std::vector<std::string> entries;
		std::string t;
		const std::string entryKey = "cue.hid_dev_battery_controller: Battery data from";
		while (std::getline(file, t))
		{
			if (t.find(entryKey) != std::string::npos)
			{
				entries.push_back(t);
			}
		}

		// Make sure we close the file, so that the iCue app can write to it.
		file.close();



		std::cout << "Found " << entries.size() << " battery level entries:" << std::endl;

		std::regex r(
			R"(^(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}.\d{3}).*cue.hid_dev_battery_controller: Battery data from ".*"\s(\d+).*$)");
		for (auto& entry : entries)
		{
			auto b = std::sregex_iterator(entry.begin(), entry.end(), r);
			auto e = std::sregex_iterator();

			for (std::sregex_iterator i = b; i != e; ++i)
			{
				std::smatch match = *i;
				if (match.size() == 3)
				{
					std::string battery;
					std::string date;
					date = match[1];
					battery = match[2];

					std::cout << "date: " << date << " -- battery: " << battery << std::endl;
				}
			}

			std::cout << std::endl;
		}

		std::cout << "Press Enter to quit" << std::endl;
		std::cin.get();
	}
	catch (std::runtime_error& e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}
}
