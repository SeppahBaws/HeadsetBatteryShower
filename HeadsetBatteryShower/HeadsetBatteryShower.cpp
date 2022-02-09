#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

using namespace std::literals;
namespace fs = std::filesystem;

void PrintVector(const std::vector<std::string>& vec)
{
	for (auto it = vec.cbegin(); it != vec.cend(); it++)
	{
		std::cout << *it << std::endl;
	}
}

int main()
{
	const std::string directory = "C:/Users/seppe/AppData/Local/Corsair/CUE4/logs";
	std::vector<fs::directory_entry> files;

	// Get all .log files in directory.
	for (const auto entry : fs::directory_iterator(directory))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".log")
		{
			files.push_back(entry);
		}
	}

	if (files.size() == 0)
	{
		throw std::exception("No .log files present in logs folder");
	}

	// Sort files based on last write time.
	std::sort(files.begin(), files.end(), [](const fs::directory_entry& left, const fs::directory_entry& right)
	{
		return left.last_write_time() > right.last_write_time();
	});

	const fs::path path = files[0].path();
	std::ifstream file(path);
	if (!file.is_open())
		throw std::exception(("Failed to open file \""s + path.string() + "\"").c_str());

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

	//PrintVector(entries);

	std::regex r("^(\\d{4}-\\d{2}-\\d{2}\\s\\d{2}:\\d{2}:\\d{2}.\\d{3}).*cue.hid_dev_battery_controller: Battery data from \".*\"\\s(\\d+).*$");
	for (auto it = entries.begin(); it != entries.end(); it++)
	{
		auto b = std::sregex_iterator((*it).begin(), (*it).end(), r);
		auto e = std::sregex_iterator();

		for (std::sregex_iterator i = b; i != e; i++)
		{
			std::smatch match = *i;
			std::string date, battery;
			if (match.size() == 3)
			{
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
