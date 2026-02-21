#include "key_loader.hpp"
#include <stdexcept>

#include "../fs.hpp"

#if __SWITCH__

#define R_THROW(x) do {				\
		Result rc = x;				\
		if (R_FAILED(rc)) {			\
			ExitServices();			\
			throw std::runtime_error(std::string("Error: ") + #x + ": " + std::to_string(rc));	\
		}	\
	} while (0)

namespace 
{
	struct NcaDecryptionkeys
	{
		bool Initialized;
		// key sources, hardcoded as they are already public.
		u8 header_key_source[0x20];
		u8 header_kek_source[0x10];
		u8 key_area_key_application_source[0x10];
		// acutal keys, derived on hardware using spl services.
		u8 header_key[0x20];
	} g_Keys = {
		.Initialized = false,
		// from https://github.com/Atmosphere-NX/Atmosphere/blob/693fb423cbbd5ab28963c5157a6f46d1aae838cf/libraries/libstratosphere/source/fssrv/fssrv_nca_crypto_configuration.cpp#L120
		.header_key_source = {
			0x5A, 0x3E, 0xD8, 0x4F, 0xDE, 0xC0, 0xD8, 0x26, 0x31, 0xF7, 0xE2, 0x5D, 0x19, 0x7B, 0xF5, 0xD0,
			0x1C, 0x9B, 0x7B, 0xFA, 0xF6, 0x28, 0x18, 0x3D, 0x71, 0xF6, 0x4D, 0x73, 0xF1, 0x50, 0xB9, 0xD2
		},
		.header_kek_source = {
			0x1F, 0x12, 0x91, 0x3A, 0x4A, 0xCB, 0xF0, 0x0D, 0x4C, 0xDE, 0x3A, 0xF6, 0xD5, 0x23, 0x88, 0x2A
		},
		.key_area_key_application_source = {
			0x7F, 0x59, 0x97, 0x1E, 0x62, 0x9F, 0x36, 0xA1, 0x30, 0x98, 0x06, 0x6F, 0x21, 0x44, 0xC3, 0x0D
		}
	};
}

hactool::ExtractionContext::ExtractionContext()
{
	R_THROW(pmdmntInitialize());
	R_THROW(splInitialize());
	R_THROW(splCryptoInitialize());
	R_THROW(fsOpenBisFileSystem(&sys, FsBisPartitionId_System, ""));

	if (fsdevMountDevice("System", sys) == -1)
		throw std::runtime_error("fsdevMountDevice");
}

hactool::ExtractionContext::~ExtractionContext()
{
	if (fsdevUnmountDevice("System") == -1)
		throw std::runtime_error("fsdevUnmountDevice");

	fsFsClose(&sys);

	pmdmntExit();
	splCryptoExit();
	splExit();
}

std::string hactool::ExtractionContext::getNcaPath(uint64_t id)
{
	char path[FS_MAX_PATH] = { 0 };
	auto rc = lrInitialize();
	if (R_FAILED(rc))
		throw std::runtime_error("lrInitialize : "s + std::to_string(rc));

	LrLocationResolver res;
	rc = lrOpenLocationResolver(NcmStorageId_BuiltInSystem, &res);
	if (R_FAILED(rc))
		throw std::runtime_error("lrOpenLocationResolver :"s + std::to_string(rc));

	rc = lrLrResolveProgramPath(&res, id, path);
	if (R_FAILED(rc))
		throw std::runtime_error("lrLrResolveDataPath : "s + std::to_string(rc));

	std::string result(path);
	result.erase(0, ((std::string)"@SystemContent://").length());
	return (std::string)"System:/Contents/" + result;
}

std::unique_ptr<hactool::ExtractionContext> hactool::Initialize()
{
	auto context = std::make_unique<ExtractionContext>();

	if (g_Keys.Initialized)
		return context;

	u8 tempheaderkek[0x10];
	Result rc = splCryptoGenerateAesKek(g_Keys.header_kek_source, 0, 0, tempheaderkek);
	if (R_FAILED(rc))
	{
		printf("splCryptoGenerateAesKek failed: %x\n", rc);
	}
	else 
	{
		rc = splCryptoGenerateAesKey(tempheaderkek, g_Keys.header_key_source, g_Keys.header_key);
		if (R_FAILED(rc))
		{
			printf("splCryptoGenerateAesKey (1) failed: %x\n", rc);
		}
		else
		{
			rc = splCryptoGenerateAesKey(tempheaderkek, g_Keys.header_key_source + 0x10, g_Keys.header_key + 0x10);

			if (R_FAILED(rc))
				printf("splCryptoGenerateAesKey (2) failed: %x\n", rc);
		}
	}

	if (R_FAILED(rc))
	{
		pritnf("Key extraction from FS failed\n");
		throw std::runtime_error("Key extraction from FS failed");
	}

	g_Keys.Initialized = true;
	return context;
}

void hactool::LoadKeys(hactool_settings_t* settings)
{
	memcpy(settings->keyset.header_key, g_Keys.header_key, sizeof(g_Keys.header_key));
	memcpy(settings->keyset.key_area_key_application_source, g_Keys.key_area_key_application_source, sizeof(g_Keys.key_area_key_application_source));
}

#else

#include <cstdio>

hactool::ExtractionContext::ExtractionContext()
{
	// Nothing to do on windows
}

hactool::ExtractionContext::~ExtractionContext()
{
	// Nothing to do on windows
}

std::string hactool::ExtractionContext::getNcaPath(uint64_t id)
{
	return SD_PREFIX "/qlaunch.nca";
}

std::unique_ptr<hactool::ExtractionContext> hactool::Initialize()
{
	FILE* f = fopen(SD_PREFIX "/prod.keys", "r");
	if (!f)
	{
		printf("prod.keys file missing\n");
		throw std::runtime_error("prod.keys file missing");
	}

	fclose(f);
	return std::make_unique<ExtractionContext>();
}

void hactool::LoadKeys(hactool_settings_t* settings)
{
	FILE* f = fopen(SD_PREFIX "/prod.keys", "r");
	extkeys_initialize_settings(settings, f);
	pki_derive_keys(&settings->keyset);
	fclose(f);
}

#endif