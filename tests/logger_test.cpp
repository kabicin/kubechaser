#include "logger/Logger.h"

#include <iostream>
#include <memory>
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

    // Messages logged before a sink exists are buffered and replayed.
    Log::Info("early ", 42);
    auto sink = std::make_shared<Logger>();
    Log::SetSink(sink);
    if (sink->Output() != "early 42\n") {
        std::cerr << "Log should replay messages logged before a sink was set.\n";
        return 1;
    }

    Log::Warning("careful");
    Log::Error("broken: ", 1.5, "\n");
    if (sink->Output() != "early 42\nWARNING: careful\nERROR: broken: 1.5\n") {
        std::cerr << "Log should prefix levels and trim trailing newlines.\n";
        return 1;
    }

    // Once the sink is gone, logging must not crash and must not reach it.
    std::weak_ptr<Logger> weakSink = sink;
    sink.reset();
    Log::Info("after sink destroyed");
    if (!weakSink.expired()) {
        std::cerr << "Log should hold its sink weakly.\n";
        return 1;
    }
    auto second = std::make_shared<Logger>();
    Log::SetSink(second);
    if (second->Output() != "after sink destroyed\n") {
        std::cerr << "Log should buffer messages while no live sink exists.\n";
        return 1;
    }
    Log::SetSink(nullptr);

    return 0;
}
