#pragma once
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <cstdlib>

namespace jd::logging
{
	enum class LogErrors : uint8_t
	{
		INFO = 0,
		WARNING = 1,
		JERROR = 2,
		FATAL = 3,
	};

	struct LoggerError : std::error_category
	{
		const char* name() const noexcept override;
		std::string message(int err) const override;

		static LoggerError& instance() {
			static LoggerError instance;
			return instance;
		}
	};

	std::error_code make_error_code(LogErrors err) noexcept;
}

namespace std
{
	template<>
	struct is_error_code_enum<::jd::logging::LogErrors> : true_type {};
}

namespace jd::logging
{
	void DefaultLogStd(std::string_view str);
	void DefaultLogFile(std::string_view str);
	void DefaultPrefixFunc(std::ostream& os, std::string_view info, int num);

	class InternalLogger
	{
	public:
		using log_prefix = void(*)(std::ostream&, std::string_view, int num);
		using log_fn = void(*)(std::string_view);

		InternalLogger() = delete;
		InternalLogger(const InternalLogger&) = delete;
		InternalLogger(InternalLogger&&) = delete;
		InternalLogger& operator=(const InternalLogger&) = delete;
		InternalLogger& operator=(InternalLogger&&) = delete;

		inline explicit InternalLogger(jd::logging::LogErrors error_type, std::string_view path, int num) 
			: error_type_(error_type) 
		{
			if (!prefix_func_ || !log_func_) [[unlikely]] {
				std::abort();
			}

			prefix_func_(stream_, path, num);
			stream_ << jd::logging::make_error_code(error_type_).message();
		}

		static void InitLog(log_fn log_func = jd::logging::DefaultLogStd, log_prefix prefix_func = jd::logging::DefaultPrefixFunc) {
			prefix_func_ = prefix_func;
			log_func_ = log_func;
		}

		inline std::ostream& stream()
		{
			return stream_;
		}

		inline ~InternalLogger() noexcept {
			stream_ << "\n";
			log_func_(stream_.str());
			if (error_type_ == LogErrors::FATAL) [[unlikely]] {
				std::terminate();
			}
		}

	private:
		LogErrors error_type_;
		std::ostringstream stream_{ std::ios_base::out };
		inline static log_fn log_func_{ nullptr };
		inline static log_prefix prefix_func_{ nullptr };
	};
}

#define LOG(severity) jd::logging::InternalLogger(jd::logging::LogErrors::##severity, __FILE__, __LINE__).stream()
#if defined(DEBUG) | defined(_DEBUG)
#define DLOG(severity) jd::logging::InternalLogger(jd::logging::LogErrors::##severity, __FILE__, __LINE__).stream()
#else
#define DLOG(sevetiry) std::ostream{nullptr}
#endif // DEBUG