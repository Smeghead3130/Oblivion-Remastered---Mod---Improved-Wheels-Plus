#include "PerfDiag.h" // TEMP-DIAG
#include "SessionContext.h"
#include "Inventory.h"

namespace sessioncontext
{
	sessionepoch::Token Current()
	{
		perfdiag::Scope timing(perfdiag::Metric::SessionCurrent); // TEMP-DIAG
		static std::mutex lock;
		static sessionepoch::Epoch epoch;
		std::scoped_lock guard(lock);
		auto* data = RE::TESDataHandler::GetSingleton();
		auto* player = RE::PlayerCharacter::GetSingleton();
		const bool available = data && !data->saveLoadGame && !data->clearingData && !data->loadingFiles && player;
		const auto seed = available ? player->characterSeed : 0u;
		return epoch.Observe({ reinterpret_cast<std::uintptr_t>(player), seed, available && inventory::Ready() });
	}
}
