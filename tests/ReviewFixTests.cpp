#include "WheelStorage.h"
#include "SessionEpoch.h"
#include "AtomicFile.h"
#include <cstdio>
#include <fstream>
#include <iterator>

int ReviewFixTests()
{
	int checks = 0, failed = 0;
	const auto check = [&](bool ok, const char* label) {
		++checks;
		if (!ok) { ++failed; std::printf("FAIL: %s\n", label); }
	};
	wheelstorage::Model model{};
	model[0][0] = { {0x1234, 0xFFFFFFFF}, 1 };
	model[1][7] = { {0xFF123456}, 0 };
	model[2][3] = { {0x4321}, -1 };
	model[3][0] = { {0x88}, 0 };
	std::ostringstream encoded;
	wheelstorage::WriteAssignments(encoded, "Hero #12345678", model);
	wheelstorage::Model decoded{};
	std::istringstream roundTrip(encoded.str());
	check(wheelstorage::ReadAssignments(roundTrip, "Hero #12345678", decoded) && decoded == model,
		"v1 round trip preserves four wheels, ordering, active and inactive slots");
	std::ostringstream header;
	wheelstorage::WriteHeader(header, "Hero #12345678");
	const std::string badRows[] = {
		"Equipment 1 active=1 nonsense", "Equipment 1 active=1 0x100000000", "Equipment 1 active=1 -1",
		"Equipment 1 active=1 0x1234junk", "Equipment 1 active=1 0x", "Equipment 1 active=1 0",
		"Equipment 0 active=1 1234", "Equipment 9 active=1 1234", "Equipment 1 active=-1 1234",
		"Equipment 1 active=2 1234", "Equipment 1 active=999999999999999999999 1234",
		"Equipment 1 active=1junk 1234", "Unknown 1 active=1 1234",
		"Magic 1 active=1 1234\nMagic 1 active=1 5678",
		"Equipment 1 active=1 1234\nMagic 2 active=1 broken"
	};
	for (const auto& row : badRows) {
		auto preserved = model;
		std::istringstream file(header.str() + row + '\n');
		check(!wheelstorage::ReadAssignments(file, "Hero #12345678", preserved) && preserved == model,
			"malformed file rejected without partial model mutation");
	}
	std::istringstream wrongCharacter(encoded.str());
	check(!wheelstorage::ReadAssignments(wrongCharacter, "Other", decoded), "identity mismatch rejected");
	std::istringstream ioFailure(encoded.str());
	ioFailure.setstate(std::ios::badbit);
	check(!wheelstorage::ReadAssignments(ioFailure, "Hero #12345678", decoded), "read failure rejected");
	std::istringstream empty(header.str());
	check(wheelstorage::ReadAssignments(empty, "Hero #12345678", decoded) && decoded == wheelstorage::Model{},
		"legacy empty setup accepted");
	std::istringstream noNewline(header.str() + "Magic 1 active=1 0Xff123456");
	check(wheelstorage::ReadAssignments(noNewline, "Hero #12345678", decoded) && decoded[1][0].entries[0] == 0xFF123456,
		"final line without newline and uppercase prefix accepted");

	sessionepoch::Epoch epoch;
	const auto unavailable = epoch.Observe({});
	const auto a = epoch.Observe({1, 42, true});
	check(!sessionepoch::Epoch::Allows(unavailable, a), "unavailable enqueue cannot dispatch");
	check(sessionepoch::Epoch::Allows(a, epoch.Observe({1, 42, true})), "normal same-character dispatch retained");
	const auto other = epoch.Observe({1, 43, true});
	check(!sessionepoch::Epoch::Allows(a, other), "same player pointer with different character seed cancels");
	const auto returned = epoch.Observe({1, 42, true});
	check(!sessionepoch::Epoch::Allows(a, returned), "return to old character does not revive work");
	const auto loading = epoch.Observe({1, 0, false});
	check(!sessionepoch::Epoch::Allows(returned, loading), "observed load interval cancels pending work");
	check(!sessionepoch::Epoch::Allows(returned, epoch.Observe({1, 42, true})), "same-character reload gets fresh epoch");
	const auto beforeReplacement = epoch.Observe({1, 42, true});
	check(!sessionepoch::Epoch::Allows(beforeReplacement, epoch.Observe({2, 42, true})), "player replacement cancels work");

	// Synthetic temporary files only. Never open installed wheel files or game saves.
	const auto dir = std::filesystem::temp_directory_path() /
		(L"iwp-storage-tests-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()));
	std::filesystem::create_directory(dir);
	const auto path = dir / L"synthetic-\u00F6.iwp";
	const auto contents = [&] {
		std::ifstream f(path, std::ios::binary);
		return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
	};
	check(atomicfile::Write(path, encoded.str()) && contents() == encoded.str(), "atomic create with Unicode path");
	const std::string large(2 * 1024 * 1024 + 19, 'x');
	check(atomicfile::Write(path, large) && contents() == large, "atomic replace writes multiple chunks exactly");
	const HANDLE locked = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	check(locked != INVALID_HANDLE_VALUE, "failure fixture locks destination against replacement");
	check(!atomicfile::Write(path, "replacement") && contents() == large, "failed replacement preserves previous bytes");
	if (locked != INVALID_HANDLE_VALUE) CloseHandle(locked);
	check(std::distance(std::filesystem::directory_iterator(dir), std::filesystem::directory_iterator{}) == 1,
		"failed replacement removes only its temporary file");
	check(!atomicfile::Write(dir / L"missing" / L"file.iwp", "data"), "temporary create failure reported");
	check(atomicfile::Write(path, "") && contents().empty(), "empty atomic replacement");
	std::filesystem::remove(path);
	std::filesystem::remove(dir);
	std::printf("Review fixes: %d checks, %d failed\n", checks, failed);
	return failed;
}
