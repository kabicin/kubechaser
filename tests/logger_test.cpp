#include "logger/Logger.h"

#include <iostream>

int main()
{
    Logger logger;

    if (!logger.Output().empty() || logger.GetState() != 0) {
        std::cerr << "A new logger should be empty.\n";
        return 1;
    }

    logger.LogMessage("first");
    logger.LogMessage("second");

    if (logger.Output() != "first\nsecond\n") {
        std::cerr << "Logger output did not preserve appended messages.\n";
        return 1;
    }
    if (logger.GetState() != 2) {
        std::cerr << "Logger state should count appended messages.\n";
        return 1;
    }

    return 0;
}
