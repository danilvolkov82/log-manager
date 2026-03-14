/**
 * @file log-builder.h
 * @brief Builder for configuring sinks and creating a logger instance.
 */
#ifndef LOG_BUILDER_H
#define LOG_BUILDER_H
#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "log-interface.h"
#include "sink-interface.h"

namespace LogManager {
/**
 * @brief Configures logging sinks and creates a single logger instance.
 *
 * The builder supports fluent configuration via @ref addSink and @ref addSinks.
 * Sinks may either be configured before registration and passed through sink
 * factories, passed directly as configured sink instances, or constructed and
 * configured by the templated @ref addSink overload. Once @ref create is
 * called, the builder is considered finalized.
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
	 * @throws std::runtime_error If the produced sink is not configured or if
	 * called after @ref create.
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
	 * @throws std::runtime_error If any produced sink is not configured or if
	 * called after @ref create.
	 */
	LogBuilder& addSinks(const std::vector<std::function<std::shared_ptr<ISink>()>>& sink_factories);

	/**
	 * @brief Adds one already constructed sink instance.
	 *
	 * The sink must not be null and must already report successful
	 * configuration through @ref ISink::isConfigured.
	 *
	 * @param sink Sink instance to register.
	 * @return Reference to this builder for fluent chaining.
	 * @throws std::invalid_argument If @p sink is null.
	 * @throws std::runtime_error If the sink is not configured or if called
	 * after @ref create.
	 */
	LogBuilder& addSink(std::shared_ptr<ISink> sink);

	/**
	 * @brief Adds multiple already constructed sink instances.
	 *
	 * Every sink must be non-null and already report successful
	 * configuration through @ref ISink::isConfigured.
	 *
	 * @param sinks Sink instances to register.
	 * @return Reference to this builder for fluent chaining.
	 * @throws std::invalid_argument If any sink is null.
	 * @throws std::runtime_error If any sink is not configured or if called
	 * after @ref create.
	 */
	LogBuilder& addSinks(const std::vector<std::shared_ptr<ISink>> &sinks);

	/**
	 * @brief Creates the configured logger.
	 * @return Shared pointer to the created logger.
	 * @throws std::runtime_error If called more than once.
	 */
	std::shared_ptr<ILog> create();

	/**
	 * @brief Constructs, configures, and registers one sink of type @p T.
	 *
	 * This overload creates the sink with `std::make_shared<T>()`, applies the
	 * provided JSON configuration, verifies that configuration succeeded through
	 * @ref ISink::isConfigured, and then registers the sink in the builder.
	 *
	 * @tparam T Concrete sink type. Must derive from @ref ISink and be default
	 * constructible.
	 * @param json_string Sink configuration payload in JSON format.
	 * @return Reference to this builder for fluent chaining.
	 * @throws std::runtime_error If sink configuration does not succeed.
	 * @throws std::runtime_error If called after @ref create.
	 */
	template<class T>
	LogBuilder& addSink(const std::string &json_string) {
		static_assert(std::is_convertible_v<T *, ISink *>, "T must be convertible to ISink");

		return this->addSink([json_string]() {
			std::shared_ptr<T> sink = std::make_shared<T>();
			sink->configure(json_string);
			if(!sink->isConfigured()) {
				throw std::runtime_error("Failed to configure sink");
			}

			return std::static_pointer_cast<ISink>(std::move(sink));
		});
	}
};
} // namespace LogManager
#endif // LOG_BUILDER_H
