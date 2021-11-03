#pragma once
#if _DEBUG
constexpr bool DebugBuild = true;
#else
constexpr bool DebugBuild = false;
#endif
#include <stdexcept>
#include <string_view>
#include <iostream>

inline void assertion(const bool& expr, const char* msg) 
{
	if constexpr (DebugBuild) {
		if (!expr) throw std::runtime_error(msg);
	}
};

enum class LogLevel
{
	Error,
	Warning,
	Info
};

enum class LogConfig
{
	Debug,
	All
};

void TENLog(std::string_view str, LogLevel level = LogLevel::Info, LogConfig config = LogConfig::Debug);
void ShutdownTENLog();
void InitTENLog();

class TENScriptException : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};