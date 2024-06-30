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
#include <chrono>
#include <format>
#include <future>
#include <vector>

namespace {
    const int chunks_of_data_to_process = 50000000;
    const size_t bufferSize = 256;
}

struct CityTemperature {

    std::string _city;
    double _min;
    double _max;
    double _sum;
    double _total;

    CityTemperature() = default;
    CityTemperature(const CityTemperature&) = default;
    CityTemperature(CityTemperature&&) = default;
    CityTemperature& operator=(const CityTemperature&) = default;



    CityTemperature(std::string const& city, double min, double max, double sum, long total) {
        _city = city;
        _min = min;
        _max = max;
        _sum = sum;
        _total = total;
    }
};

using CityMap = std::unordered_map<std::string, CityTemperature>;

bool caseInsensitiveCharCompare(char c1, char c2) {
    return std::tolower(c1) == std::tolower(c2);
}

bool equalsIgnoreCase(const std::string& str1, const std::string& str2) {
    return str1.size() == str2.size() &&
        std::equal(str1.begin(), str1.end(), str2.begin(), caseInsensitiveCharCompare);
}

CityMap calculate_min_avg_max_temp(FILE* file, bool debug_flag ) {
    
    char buffer[bufferSize];
    CityMap cityMap;
    long rows = 0;

    while (fgets(buffer, bufferSize, file)) {
        size_t last_index = strcspn(buffer, "\n");
        buffer[last_index] = 0;

        auto found = std::find(buffer, buffer + last_index, ';');
        auto city = std::string(buffer, found);
        double temperature = std::stod(std::next(found));



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
                std::cout << rows << " completed\n";
            }
        }
    }
    return cityMap;
}

void print_output(CityMap cityMap, std::chrono::steady_clock::time_point start) {
    std::cout << "{";
    std::map<std::string, CityTemperature> tempMap{ cityMap.begin(), cityMap.end() };
    std::string outputMsg;
    for (auto const& [city, value] : tempMap) {
        outputMsg += std::format("{}={:.1f}/{:.1f}/{:.1f}, ", city, value._min, (value._sum / value._total), value._max);
    }
    //std::cout << outputMsg.substr(0, outputMsg.rfind(", ")) << "}\n";
    std::cout << outputMsg << "}\n";

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    long hours = (long)elapsed / 3600;
    long minutes = (long)(elapsed % 3600) / 60;
    long seconds = (long)elapsed % 60;
    std::cout << std::setw(2) << std::setfill('0') << hours << ":"
              << std::setw(2) << std::setfill('0') << minutes << ":"
              << std::setw(2) << std::setfill('0') << seconds << std::endl;
}


int main( int argc, char** argv)
{    
    FILE* file = fopen(argv[1], "r");
    if (!file) {
        std::perror("Error opening file");
        return 1;
    }

    const bool debug_flag = (argc >= 3 && equalsIgnoreCase( argv[2], "debug"));
    
    auto start = std::chrono::high_resolution_clock::now();
 
    print_output(calculate_min_avg_max_temp(file, debug_flag), start);

    

    fclose(file);
}