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

void print_error(std::string errorMessage,
                std::string functionName = __builtin_FUNCTION(),
                unsigned int lineNumber = __builtin_LINE());

void print_warning(std::string warningMessage,
                std::string functionName = __builtin_FUNCTION(),
                unsigned int lineNumber = __builtin_LINE());

void print_success(std::string successMessage);

void print_current_directory();

int random_integer(int minimum, int maximum);

float random_float(float minimum, float maximum);