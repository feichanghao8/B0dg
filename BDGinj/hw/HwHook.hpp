#pragma once

#include <filesystem>

namespace hw {
// Sets the path of the hardware profile to be used when queried by the game
bool SetProfile(const std::filesystem::path &filepath);

// Initialize (using MinHook) the hooks 
void CreateHooks();
}  // namespace hw
