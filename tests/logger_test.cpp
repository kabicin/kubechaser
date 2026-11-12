#include "logger/Logger.h"

#include <iostream>
#include <string>

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

    Logger capped(3);
    for (int i = 0; i < 5; ++i) {
        capped.LogMessage("m" + std::to_string(i));
    }
    if (capped.Output() != "m2\nm3\nm4\n") {
        std::cerr << "Logger should keep only the newest maxLines messages.\n";
        return 1;
    }
    if (capped.GetState() != 5) {
        std::cerr << "Logger state should keep counting after trimming.\n";
        return 1;
    }

    return 0;
}
