#include "HwHook.hpp"

#include "../nlohmann/json.hpp"
#include "../minhook/MinHook.h"
#include "../encryption/base64.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <regex>
#include <vector>

#include <Windows.h>

namespace hw {
namespace {

// The loaded hardware profile
nlohmann::json gHwProfile {};

// Returns the bbox data of the cloaked hardware profile in base64 encoding
static std::string GetBBKeyDataBase64()
{
	const auto &templ = gHwProfile["template"];
	const auto &sp = gHwProfile["sp"];

	std::string cspUUID = sp["computer_system_product_uuid"];
	cspUUID.erase(std::remove(cspUUID.begin(), cspUUID.end(), '-'), cspUUID.end()); // Remove "-"
	std::transform(cspUUID.begin(), cspUUID.end(), cspUUID.begin(), [](char c) { return std::toupper(c); }); // To uppercase

	std::string macAddress = sp["mac_address"];
	macAddress.erase(std::remove(macAddress.begin(), macAddress.end(), ':'), macAddress.end()); // Remove ":"
	std::transform(macAddress.begin(), macAddress.end(), macAddress.begin(), [](char c) { return std::tolower(c); }); // To lowercase

	std::vector<uint8_t> data {};

	const auto addString = [&data](const std::string &str)
	{
		for (char c : str) data.emplace_back(static_cast<unsigned char>(c));
		data.emplace_back(5);
	};
	const auto addStringNoSep = [&data](const std::string &str)
	{
		for (char c : str) data.emplace_back(static_cast<unsigned char>(c));
	};

	addString("WMIKEY");
	addString(templ["bios"]["SerialNumber"]);
	addString(cspUUID);
	addString(templ["baseboard"]["SerialNumber"]);
	addString(templ["bios"]["SerialNumber"]);
	addString(templ["system_enclosure"]["SMBIOSAssetTag"]);
	addString(templ["processor"]["ProcessorId"]);
	addString("CPU");
	addString(templ["disk_drive"]["SerialNumber"]);
	addString(macAddress);
	addString(templ["video_controller_int"]["DriverDate"]);
	addString(templ["video_controller_int"]["DriverVersion"]);
	addString("0"); // ProductId
	addString("0"); // InstallDate
	addString("0"); // DigitalProductId
	addString("ffffffff-ffff-ffff-ffff-ffffffffffff"); // BuildGUID
	addString("0"); // ClientHWID
	addString(templ["inetcomm_version"]);
	addString(templ["inetexplorer_version"]);
	addStringNoSep(std::format("{}", templ["k32_GetVersion"].get<uint32_t>()));

	return base64_encode(data);
}

static void HandeBBKeyReplacement(void *buffer, size_t len, uint32_t *lenRead)
{
	const char *const textBuffer = reinterpret_cast<const char*>(buffer);

	// Assume buffer as UTF-8, and search for the bbox output signature
	if (strstr(textBuffer, "BBKEY") && (strstr(textBuffer, "resetApp") == nullptr))
	{
		static const std::regex regex {"BBKEY:\\s*([A-Za-z0-9+/=]+)"};
		const std::string newBuffer = std::regex_replace(textBuffer, regex, "BBKEY: " + GetBBKeyDataBase64());

		if (newBuffer.size() > len)
		{
			// Unlikely case since the HW information isn't that big, but if this does happen we would need
			// to batch the write-backs.
			MessageBoxA(nullptr, "Not enough space allocated by caller to replace the HW buffer\n\nExtra implementation may be required.",
				"Injection Error", MB_ICONERROR | MB_OK);
			TerminateProcess(reinterpret_cast<HANDLE>(-1), -1);
		}

		memcpy(buffer, newBuffer.data(), newBuffer.size() + 1);
		*lenRead = newBuffer.size() + 1;

		printf("BBox buffer: %s\n", textBuffer);
	}
}

}  // namespace

bool SetProfile(const std::filesystem::path &filepath)
{
	std::ifstream profileStream {filepath};
	
	gHwProfile = nlohmann::json::parse(profileStream, nullptr, false);
	return !gHwProfile.is_discarded();
}

BOOL(__stdcall *gOriginalReadFile)(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
) {};
static BOOL __stdcall HookedReadFile(
	HANDLE       hFile,
	LPVOID       lpBuffer,
	DWORD        nNumberOfBytesToRead,
	LPDWORD      lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped
)
{
	const BOOL result = gOriginalReadFile(
		hFile,
		lpBuffer,
		nNumberOfBytesToRead,
		lpNumberOfBytesRead,
		lpOverlapped
	);

	if (result && lpBuffer && lpNumberOfBytesRead && *lpNumberOfBytesRead > 0)
	{
		const char *const textBuffer = reinterpret_cast<const char*>(lpBuffer);

		__try
		{
			HandeBBKeyReplacement(lpBuffer, nNumberOfBytesToRead, reinterpret_cast<uint32_t*>(lpNumberOfBytesRead));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			printf("HookedReadFile threw but was caught!\n");
		}
	}

	return result;
}

void CreateHooks()
{
	const FARPROC api = GetProcAddress(GetModuleHandleA("kernelbase"), "ReadFile");
	MH_CreateHook(api, HookedReadFile, reinterpret_cast<void**>(&gOriginalReadFile));
}
}  // namespace hw
