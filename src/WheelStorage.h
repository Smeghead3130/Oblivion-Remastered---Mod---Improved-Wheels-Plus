#pragma once

#include <cctype>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>

namespace wheelstorage
{
	inline constexpr std::string_view kSignature = "# ImprovedWheelsPlus assignments v1";
	inline constexpr std::string_view kSpellNamesSignature = "# ImprovedWheelsPlus spell names v1";   // <character>.spells, same folder
	inline constexpr std::string_view kCharacterPrefix = "# Character: ";

	// the file stem of a character: what a file name may keep of the identity (the header carries the exact identity)
	inline std::string SafeName(std::string_view a_name)
	{
		std::string safe;
		for (const char c : a_name) {
			safe += (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '-' || c == '_') ? c : '_';
		}
		return safe;
	}

	inline bool ReadHeader(std::istream& a_in, std::string_view a_character, std::string_view a_signature = kSignature)
	{
		std::string signature, character;
		return std::getline(a_in, signature) && std::getline(a_in, character) &&
			signature == a_signature && character == std::string(kCharacterPrefix) + std::string(a_character);
	}

	inline void WriteHeader(std::ostream& a_out, std::string_view a_character, std::string_view a_signature = kSignature)
	{
		a_out << a_signature << '\n' << kCharacterPrefix << a_character << '\n';
	}
}
