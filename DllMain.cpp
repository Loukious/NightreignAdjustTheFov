#include <Windows.h>
#include <xmmintrin.h>

#include "ModUtils.h"

using namespace ModUtils;
using namespace mINI;

static float fovValue = 48.0f;

void ReadConfig()
{
	INIFile config(GetModFolderPath() + "\\config.ini");
	INIStructure ini;

	if (config.read(ini))
	{
		fovValue = std::stof(ini["fov"].get("value"));
	} 
	else
	{
		ini["fov"]["value"] = "48";
		config.write(ini, true);
	}

	Log("Field of view: ", fovValue);
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
	Log("Activating AdjustTheFov...");
	const std::string aob = "e9 ? ? ? ? 48 8b 01 48 85 c0 74 06 f3 0f 10 40 14 c3 0f 57 c0 c3";
	uintptr_t hookAddress = 0;
	const int offset = 5;
	ReadConfig();

	std::vector<unsigned char> fovBytes(sizeof(float), 0);
	MemCopy((uintptr_t)&fovBytes[0], (uintptr_t)&fovValue, sizeof(float));
	std::string fovHexStr = RawAobToStringAob(fovBytes);
	const std::string patchedBytes = "b8 " + fovHexStr + " 66 0f 6e c0 c3 90 90 90 90 90 90 90 90";

	hookAddress = AobScan(aob);

	if (hookAddress == 0)
	{
		Log("AdjustTheFov pattern not found!");
		return 1;
	}

	hookAddress += offset;

	ReplaceExpectedBytesAtAddress(hookAddress, aob.substr(11), patchedBytes);

	CloseLog();
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
		CreateThread(0, 0, &MainThread, 0, 0, NULL);
	}
	return 1;
}