#include "jd_types.hpp"

#include <utility>
#include <variant>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <functional>
#include <unordered_map>

namespace jd::utils
{
	template<typename Opts>
	struct ConfigParser : Opts
	{
		using vars_t = std::variant<std::string Opts::*, int32_t Opts::*, uint32_t Opts::*, double Opts::*, bool Opts::*>;
		using value_t = std::pair<std::string, vars_t>;

		ConfigParser(const ConfigParser&) = delete;
		ConfigParser(ConfigParser&&) = delete;
		ConfigParser& operator=(const ConfigParser&) = delete;
		ConfigParser& operator=(ConfigParser&&) = delete;

		Opts parse(std::string_view sv)
		{
			for (auto& [token, cb] : callbacks_) {
				size_t pos_start = sv.find(token.c_str());

				// We are did not find neccessary token
				if (pos_start == sv.npos) {
					continue;
				}

				pos_start += token.size();
				pos_start = removeSpaces(sv, pos_start);

				if (pos_start == sv.npos || sv[pos_start] != '=') {
					continue;
				}

				pos_start++;
				pos_start = removeSpaces(sv, pos_start);

				if (pos_start == sv.npos) {
					continue;
				}

				size_t pos_end = sv.find(';', pos_start);
				if (pos_end == sv.npos || sv[pos_end] != ';') {
					continue;
				}

				cb(sv.substr(pos_start, pos_end - pos_start));
			}

			return static_cast<Opts>(*this);
		}

		static uptr<ConfigParser> make_unique(std::initializer_list<value_t> args)
		{
			auto parser = uptr<ConfigParser>(new ConfigParser); // use "new" because of private constructor

			for (auto arg : args) {
				parser->registerCallback(arg);
			}

			return parser;
		}

	private:
		using callback_t = std::function<void(std::string_view)>;
		std::unordered_map<std::string, callback_t> callbacks_;

		ConfigParser() = default;

		auto registerCallback(const std::string& name, vars_t vars)
		{
			callbacks_[name] = [this, name, vars](std::string_view value) {
				std::visit(
					[this, &value](auto& arg) {
						std::stringstream parsed_value;
						parsed_value << value;
						parsed_value >> this->*arg;
					},
					vars);
				};
		}

		auto registerCallback(const value_t& value) {
			return registerCallback(value.first, value.second);
		}

		static size_t removeSpaces(std::string_view sv, size_t pos)
		{
			while (pos < sv.size() && sv[pos] == ' ') {
				pos++;
			}
			return pos == sv.size() ? sv.npos : pos;
		}
	};
}