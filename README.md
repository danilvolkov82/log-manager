# log-manager

`log-manager` is a C++ logging library built around a logger interface,
configurable sinks, and a fluent `LogBuilder`.

Public include layout:

- Installed headers keep the source layout under `log-manager/sinks/...`.
- `log-manager.h` aggregates the root public headers.
- `file-sink.h`, `console-sink.h`, and `system-log-sink.h` are generated convenience umbrellas.

## LogBuilder

`LogBuilder` supports three sink registration styles:

- Register an already configured sink through a factory.
- Register an already configured sink instance directly.
- Construct and configure a sink directly through the templated `addSink<T>()`
  overload.

Example:

```cpp
#include "log-builder.h"
#include "sinks/console-sink/console-sink.h"

using namespace LogManager;
using LogManager::Sinks::ConsoleSink::ConsoleSink;

LogBuilder builder;

builder.addSink<ConsoleSink>(R"({
    "message_format": "[{level}] {message}",
    "log_level": "INFO"
})");

auto logger = builder.create();
logger->info("app", "started");
```

The direct `addSink(std::shared_ptr<ISink>)` overload expects an already
configured sink instance and rejects null or unconfigured sinks.

The templated `addSink<T>()` overload:

- default-constructs the sink
- applies the provided JSON configuration
- verifies successful configuration via `isConfigured()`
- throws if configuration does not succeed
