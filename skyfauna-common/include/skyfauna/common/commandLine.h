// SPDX-License-Identifier: MIT
// Copyright (C) 2025 Chloe Eather

#ifndef SKYFAUNA_COMMON_GETOPT_H_
#define SKYFAUNA_COMMON_GETOPT_H_

#include <skyfauna/common/util.h>

#include <any>
#include <array>
#include <functional>
#include <span>
#include <string_view>
#include <vector>
#include <cstdint>
#include <cstring>

namespace skyfauna::common {
enum class CmdLineArgRequired : uint8_t {
	NO_ARG = 0,
	OPTIONAL_ARG,
	REQUIRED_ARG
};

enum class CmdLineArgType : uint8_t {
	FLAG = 0, INT, STRING
};

template<typename E>
requires std::is_enum_v<E>
class CommandLineParser;

template<typename Id>
struct CmdLineOption {
	std::string_view longName;
	char shortName;
	CmdLineArgRequired req;
	Id identifier;

	CmdLineOption() = default;
	CmdLineOption(std::string_view n, char s, CmdLineArgRequired r, Id i)
	 : longName(n), shortName(s), req(r), identifier(i), hash(Hash(longName))
	{}
private:
	std::size_t hash;
	friend class CommandLineParser<Id>;
};

template<typename Id>
struct CmdLineArg {
	CmdLineArgType type;
	Id identifier;
	std::any arg;
};

template<typename E>
requires std::is_enum_v<E>
class CommandLineParser {
public:
	using enum_type = E;
	using callback_type = std::function<void(const CmdLineArg<enum_type>&)>;

public:
	inline CommandLineParser(int argc, char *argv[]) noexcept
	 : m_Command(argv[0]),
		m_CmdLine(const_cast<const char **>(&argv[1]),
		 const_cast<const char **>(&argv[argc]))
	{}

	template<typename InputIt>
	requires std::input_iterator<InputIt>
	inline void SetLongOpts(InputIt begin, InputIt end) {
		m_Opts.resize(std::distance(begin, end));
		std::copy(begin, end, m_Opts.begin());
	}

	inline void SetLongOpts(const std::vector<CmdLineOption<enum_type>>& opts) {
		SetLongOpts(opts.begin(), opts.end());
	}

	template<std::size_t N>
	inline void SetLongOpts(const std::array<CmdLineOption<enum_type>, N>& opts) {
		SetLongOpts(opts.begin(), opts.end());
	}

	template<std::size_t N = std::dynamic_extent>
	inline void SetLongOpts(const std::span<CmdLineOption<enum_type>, N> opts) {
		SetLongOpts(opts.begin(), opts.end());
	}

	void Parse(const callback_type& func = callback_type()) {
		int adv = 1;
		for (auto it = m_CmdLine.begin();
			it != m_CmdLine.end();
			it = std::ranges::next(it, adv, m_CmdLine.end()))
		{
			adv = 1;
			const char *arg = *it;
			if (arg[0] != '-') {
				m_UnhandledArgs.push_back(arg);
				continue;
			}
			std::size_t argLen = std::strlen(arg);
			if (argLen < 2) {
				m_UnhandledArgs.push_back(arg);
				continue;
			}
			std::string_view nextArg = "";
			if (it + 1 != m_CmdLine.end())
				nextArg = *(it + 1);
			if (MatchShortOpt(arg, nextArg, adv) || MatchLongOpt(arg, nextArg, adv)) {
				if (func)
					func(m_Flags.back());
			} else {
				m_UnhandledArgs.push_back(arg);
			}
		}
	}

	std::vector<CmdLineArg<enum_type>>& GetFlags() { return m_Flags; }
	const std::vector<CmdLineArg<enum_type>>& GetResults() const { return m_Flags; }
	std::vector<std::string_view>& GetUnhandledArgs() { return m_UnhandledArgs; }
	const std::vector<std::string_view>& GetUnhandledArgs() const { return m_UnhandledArgs; }
	const std::string_view& GetCommand() const { return m_Command; }

private:
	void ParseParam(const std::string_view param, enum_type identifier) {
		bool number = true;
		std::for_each(param.begin(), param.end(),
				[&number](char c){ if (!std::isdigit(c)) number = false; });
		if (number) {
			m_Flags.emplace_back(CmdLineArgType::INT,
						identifier,
						std::atoi(param.data()));
		} else {
			m_Flags.emplace_back(CmdLineArgType::STRING,
						identifier,
						std::string(param));
		}
	}

	bool MatchShortOpt(const std::string_view arg,
					const std::string_view next,
					int& adv)
	{
		auto pred = [arg, next](const CmdLineOption<enum_type>& opt) -> bool {
			if (opt.shortName == arg[1]) {
				switch (opt.req) {
					case CmdLineArgRequired::NO_ARG:
						if (arg.length() == 2)
							return true;
						break;

					case CmdLineArgRequired::OPTIONAL_ARG:
						return true;

					case CmdLineArgRequired::REQUIRED_ARG:
						if (arg.length() > 2 || next.length() > 0)
							return true;
						break;

					default:
						throw std::logic_error("Invalid argument required flag");
				}
			}
			return false;
		};
		auto it = std::find_if(m_Opts.begin(), m_Opts.end(), pred);
		if (it == m_Opts.end())
			return false;

		const CmdLineOption<enum_type>& match = *it;
		if (match.req == CmdLineArgRequired::NO_ARG) {
			m_Flags.emplace_back(CmdLineArgType::FLAG, match.identifier);
			return true;
		}
		std::string_view param;
		if (arg.length() > 2) {
			param = arg.substr(2);
		} else {
			param = next;
			++adv;
		}
		if (param.length() == 0)
			return false;

		ParseParam(param, match.identifier);

		return true;
	}

	bool MatchLongOpt(const std::string_view arg,
				   const std::string_view next,
				   int& adv)
	{
		if (arg[1] != '-') {
			return false;
		}
		auto eqPos = arg.find('=');
		std::string_view argName = arg.substr(2);
		std::string_view param;
		if (eqPos == arg.npos) {
			param = next;
		} else {
			param = arg.substr(eqPos + 1);
			argName = arg.substr(2, eqPos - 2);
		}
		std::size_t thisArgHash = Hash(argName);
		auto it = std::find_if(m_Opts.begin(), m_Opts.end(),
							 [thisArgHash](const CmdLineOption<enum_type>& opt) -> bool {
								return opt.hash == thisArgHash;
							 });
		if (it == m_Opts.end())
			return false;

		if (it->req != CmdLineArgRequired::NO_ARG && eqPos == arg.npos) {
			++adv;
		}

		ParseParam(param, it->identifier);
		return true;
	}

private:
	const std::string_view m_Command;
	const std::span<const char *> m_CmdLine;

	std::vector<CmdLineOption<enum_type>> m_Opts;
	std::vector<CmdLineArg<enum_type>> m_Flags;
	std::vector<std::string_view> m_UnhandledArgs;
};
}

#endif

