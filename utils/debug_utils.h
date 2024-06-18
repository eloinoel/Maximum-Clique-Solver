#pragma once

#include <string>
#include <stdexcept>

/**
 * @brief Custom exception.
 */
class NotImplemented : public std::logic_error
{
public:
    NotImplemented() : std::logic_error("Function not yet implemented") { };
};

#define assertTrue(exp, msg) assert(((void)msg, exp))
#define assertFalse(exp, msg) assert(((void)msg, !exp))

void printError(std::string errorMessage,
                std::string functionName = __builtin_FUNCTION(),
                unsigned int lineNumber = __builtin_LINE());

void printWarning(std::string warningMessage,
                std::string functionName = __builtin_FUNCTION(),
                unsigned int lineNumber = __builtin_LINE());

void printSuccess(std::string successMessage);

void printCurrentDirectory();

int randomInteger(int minimum, int maximum);

float randomFloat(float minimum, float maximum);