#include "colors.h"
#include "debug_utils.h"

#include <filesystem>
#include <float.h>
#include <random>

void print_error(std::string errorMessage, std::string functionName, unsigned int lineNumber)
{
    std::cout
    << RED
    << "Error in " + functionName + ", " + std::to_string(lineNumber) + ": "
    << RESET
    << RED_BRIGHT
    << errorMessage
    << RESET
    << std::endl;
}

void print_warning(std::string warningMessage, std::string functionName, unsigned int lineNumber)
{
    std::cout
    << YELLOW
    << "Warning from " + functionName + ", " + std::to_string(lineNumber) + ": "
    << RESET
    << YELLOW_BRIGHT
    << warningMessage
    << RESET
    << std::endl;
}

void print_success(std::string successMessage)
{
    std::cout
    << GREEN
    << successMessage
    << RESET
    << std::endl;
}

void print_current_directory()
{
    std::filesystem::path cwd = std::filesystem::current_path() / "filename.txt";
    std::cout << "Current Directory: " << cwd.string() << std::endl;
}

int random_integer(int minimum, int maximum) {
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<std::mt19937::result_type> distr(minimum, maximum);

    return distr(rng);
}

float random_float(float minimum, float maximum){
    return minimum + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maximum-minimum)));
}