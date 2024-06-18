#include "colors.h"
#include "debug_utils.h"

#include <iostream>
#include <filesystem>
#include <float.h>
#include <random>

void printError(std::string errorMessage, std::string functionName, unsigned int lineNumber)
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

void printWarning(std::string warningMessage, std::string functionName, unsigned int lineNumber)
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

void printSuccess(std::string successMessage)
{
    std::cout
    << GREEN
    << successMessage
    << RESET
    << std::endl;
}

void printCurrentDirectory()
{
    std::filesystem::path cwd = std::filesystem::current_path() / "filename.txt";
    std::cout << "Current Directory: " << cwd.string() << std::endl;
}

int randomInteger(int minimum, int maximum) {
    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<std::mt19937::result_type> distr(minimum, maximum);

    return distr(rng);
}

float randomFloat(float minimum, float maximum){
    return minimum + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(maximum-minimum)));
}