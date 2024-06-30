/*
*** Attempting to solve 1 billion row challenge. This is a baseline attempt in making the code faster than the baseline java code.
*
*/



#include <iostream>
#include<fstream>
#include <string>
#include <tuple>
#include <string_view>
#include <charconv>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <format>
#include <future>
#include <vector>

namespace {

	const int chunks_of_data_to_process = 50000000;
	const size_t bufferSize = 256;
	const size_t precision = 1;

	void print_time_elapsed(std::chrono::steady_clock::time_point const& start, std::chrono::steady_clock::time_point const& end) {
		auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
		long hours = (long)elapsed / 3600;
		long minutes = (long)(elapsed % 3600) / 60;
		long seconds = (long)elapsed % 60;
		std::cout << "Elapsed: ";
		std::cout << std::setw(2) << std::setfill('0') << hours << ":"
			<< std::setw(2) << std::setfill('0') << minutes << ":"
			<< std::setw(2) << std::setfill('0') << seconds << '\n';
	}

	long get_temperature(const char* buffer, const char* iter_offset, const size_t last_index) {
		auto till_decimal = std::find(std::next(iter_offset), buffer + last_index, '.');
		long temp = std::stol(std::next(iter_offset));

		long decimal_points = *(std::next(till_decimal)) - '0';

		if (temp < 0) {
			temp = (-1 * temp * std::pow(10, precision)) + decimal_points;
			temp *= -1;
		}
		else {

			temp = (temp * (std::pow(10, precision)) + (decimal_points));
		}
		return temp;
	}

	bool caseInsensitiveCharCompare(char c1, char c2) {
		return std::tolower(c1) == std::tolower(c2);
	}

	bool equalsIgnoreCase(const std::string& str1, const std::string& str2) {
		return str1.size() == str2.size() &&
			std::equal(str1.begin(), str1.end(), str2.begin(), caseInsensitiveCharCompare);
	}

	/* 
	   Not used for now.
	*/
	// Custom hash function for string*
	struct StringPtrHash {
		std::size_t operator()(const std::string_view& s) const {
			return std::hash<std::string>()(s.data());
		}
	};

	// Custom equality function for string*
	struct StringPtrEqual {
		bool operator()(const std::string_view& lhs, const std::string_view& rhs) const {
			return strcmp(lhs.data(), rhs.data());
		}
	};
}

struct CityTemperature {

	std::string _city;
	long _min;
	long _max;
	long _sum;
	long _total;

	CityTemperature() = default;
	CityTemperature(const CityTemperature&) = default;
	CityTemperature(CityTemperature&&) = default;
	CityTemperature& operator=(const CityTemperature&) = default;



	CityTemperature(std::string city, long min, long max, long sum, long total) {
		_city = city;
		_min = min;
		_max = max;
		_sum = sum;
		_total = total;
	}
};

using CityMap = std::unordered_map<std::string, CityTemperature>;

CityMap calculate_min_avg_max_temp(FILE* file, bool debug_flag) {

	char buffer[bufferSize];
	CityMap cityMap(100);
	long rows = 0;
	
	auto start = std::chrono::high_resolution_clock::now();

	while (fgets(buffer, bufferSize, file)) {

		size_t last_index = strcspn(buffer, "\n");
		buffer[last_index] = 0;
		auto found = std::find(buffer, buffer + last_index, ';');
		std::string city{ buffer, found };

		long temperature = get_temperature(buffer, found, last_index);

		auto key_found = cityMap.find(city);

		if (key_found != cityMap.end()) {
			CityTemperature& cityTemp = key_found->second;
			cityTemp._max = std::max(cityTemp._max, temperature);
			cityTemp._min = std::min(cityTemp._min, temperature);
			cityTemp._sum += temperature;
			cityTemp._total += 1;
		}
		else {
			cityMap.insert(cityMap.end(), std::make_pair(city, CityTemperature(city, temperature, temperature, temperature, 1)));
		}

		if (debug_flag) {
			++rows;
			if (rows % chunks_of_data_to_process == 0) {
				std::cout << std::format("{} completed.\n", rows);
				print_time_elapsed(start, std::chrono::high_resolution_clock::now());
				start = std::chrono::high_resolution_clock::now();
			}
		}
	}
	return cityMap;
}



void print_output(const CityMap& cityMap, std::chrono::steady_clock::time_point start) {
	std::cout << "{";
	std::map<std::string_view, CityTemperature> tempMap{ cityMap.begin(), cityMap.end() };
	std::string outputMsg;

	char buffer[bufferSize] = { '\0' };
	for (auto const& [city, value] : tempMap) {
		double min_temp = static_cast<double>(value._min) / 10;
		double avg_temp = (static_cast<double>(value._sum) / static_cast<double>(value._total)) / 10;
		double max_temp = static_cast<double>(value._max) / 10;
		sprintf(buffer, "%s=%.1f/%.1f/%.1f, ", city.data(), min_temp, avg_temp, max_temp);
		outputMsg += buffer;
	}
	std::cout << outputMsg.substr(0, outputMsg.rfind(", ")) << "}\n";

	auto end = std::chrono::high_resolution_clock::now();
	print_time_elapsed(start, end);
}


int main(int argc, char** argv)
{
	FILE* file = fopen(argv[1], "r");
	if (!file) {
		std::perror("Error opening file");
		return 1;
	}

	const bool debug_flag = (argc >= 3 && equalsIgnoreCase(argv[2], "debug"));

	auto start = std::chrono::high_resolution_clock::now();
	print_output(calculate_min_avg_max_temp(file, debug_flag), start);

	fclose(file);
}