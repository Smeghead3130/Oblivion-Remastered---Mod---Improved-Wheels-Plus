#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <string_view>

namespace wheelstorage
{
	inline constexpr std::string_view kSignature = "# ImprovedWheelsPlus assignments v1";
	inline constexpr std::string_view kCharacterPrefix = "# Character: ";

	inline bool ReadHeader(std::istream& a_in, std::string_view a_character)
	{
		std::string signature, character;
		return std::getline(a_in, signature) && std::getline(a_in, character) &&
			signature == kSignature && character == std::string(kCharacterPrefix) + std::string(a_character);
	}

	inline void WriteHeader(std::ostream& a_out, std::string_view a_character)
	{
		a_out << kSignature << '\n' << kCharacterPrefix << a_character << '\n';
	}
}
