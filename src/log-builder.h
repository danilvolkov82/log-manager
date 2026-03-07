/**
 * @file log-builder.h
 * @brief Builder for configuring sinks and creating a logger instance.
 */
#ifndef LOG_BUILDER_H
#define LOG_BUILDER_H
#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "log-interface.h"
#include "sink-interface.h"

namespace LogManager {
/**
 * @brief Configures logging sinks and creates a single logger instance.
 *
 * The builder supports fluent configuration via @ref addSink and @ref addSinks.
 * Sinks are expected to be configured before they are registered in the
 * builder. Once @ref create is called, the builder is considered finalized.
 */
class LogBuilder {
private:
	class Impl;
	std::unique_ptr<Impl> _impl;
public:
	/**
	 * @brief Constructs an empty builder.
	 */
	LogBuilder();

	/**
	 * @brief Destroys the builder.
	 */
	~LogBuilder();

		/**
		 * @brief Adds a single sink produced by the given factory.
		 *
		 * The returned sink instance is expected to have already completed its
		 * single allowed configuration step.
		 *
		 * @param sink_factory Callable that returns a non-null sink instance.
		 * @return Reference to this builder for fluent chaining.
		 * @throws std::invalid_argument If the factory is empty or returns null.
		 * @throws std::runtime_error If called after @ref create.
		 */
	LogBuilder& addSink(std::function<std::shared_ptr<ISink>()> sink_factory);

		/**
		 * @brief Adds multiple sinks from the provided factories.
		 *
		 * Every produced sink instance is expected to have already completed its
		 * single allowed configuration step.
		 *
		 * @param sink_factories Collection of factories used to create sinks.
		 * @return Reference to this builder for fluent chaining.
		 * @throws std::invalid_argument If any factory is empty or returns null.
		 * @throws std::runtime_error If called after @ref create.
	 */
	LogBuilder& addSinks(const std::vector<std::function<std::shared_ptr<ISink>()>>& sink_factories);

	/**
	 * @brief Creates the configured logger.
	 * @return Shared pointer to the created logger.
	 * @throws std::runtime_error If called more than once.
	 */
	std::shared_ptr<ILog> create();
};
} // namespace LogManager
#endif // LOG_BUILDER_H
