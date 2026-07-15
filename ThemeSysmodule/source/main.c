#include <switch.h>
#include <string.h>

// We try to have the smallest impact on memory as possible.
// By default, we don't use the heap at all.
#define USE_HEAP 0

#if !USE_HEAP
void* __libnx_aligned_alloc(size_t alignment, size_t size)
{
	return NULL;
}

void __libnx_free(void* p)
{

}

void __libnx_initheap(void)
{

}
#else

#define INNER_HEAP_SIZE (1 * 1024 * 1024)

size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char nx_inner_heap[INNER_HEAP_SIZE];

void __libnx_initheap(void)
{
	void* addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	// Newlib
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end = (char*)addr + size;
}
#endif

u32 __nx_applet_type = AppletType_None;
u32 __nx_fs_num_sessions = 1;
u32 __nx_fsdev_direntry_cache_size = 1;

#define QLAUNCH_ID 0x0100000000001000

static FsFileSystem sdCard;
static Event homeMenuLaunched;
static SetSysFirmwareVersion fw;

volatile char version_tag[] = "THEME_SYMODULE:2=";

void __attribute__((weak)) __appInit(void)
{
	Result rc;

	rc = smInitialize();
	if (R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
	
	// pmdmnt was changed with fw updates, libnx needs to know hos version to call the correct functions.
	rc = setsysInitialize();
	if (R_SUCCEEDED(rc)) {
		rc = setsysGetFirmwareVersion(&fw);
		if (R_FAILED(rc)) fatalThrow(rc);
			
		hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
		setsysExit();
	}

	rc = pmdmntInitialize();
	if (R_FAILED(rc))
		fatalThrow(rc);

	// Try to do this as fast as possible
	// In ams source code, the create process hook is a global variable
	// This means only one hook can be set at a time and this function fails if it has already been done.
	// In practice this is unlikely because this function is rarely used and our hook fires early during boot since qlaunch always launches.
	rc = pmdmntHookToCreateProcess(&homeMenuLaunched, QLAUNCH_ID);
	if (R_FAILED(rc))
		fatalThrow(rc);

	rc = fsInitialize();
	if (R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

	// We use this directly because we don't want to depend on fsdev which requires malloc
	rc = fsOpenSdCardFileSystem(&sdCard);
	if (R_FAILED(rc))
		fatalThrow(rc);
}

void __attribute__((weak)) __appExit(void)
{
	fsFsClose(&sdCard);
	fsExit();
	eventClose(&homeMenuLaunched);
	pmdmntExit();
	smExit();
}

#define SECONDS(x) ((u64)(x)*1000000000ULL)

static bool should_delete_themes() 
{
	FsFile versionFile;

	if (R_FAILED(fsFsOpenFile(&sdCard, "/atmosphere/contents/0100000000001000/version_hash.bin", FsOpenMode_Read, &versionFile)))
	{
		FsDir dir;
		if (R_FAILED(fsFsOpenDirectory(&sdCard, "/atmosphere/contents/0100000000001000/romfs", FsDirOpenMode_ReadFiles | FsDirOpenMode_NoFileSize, &dir)))
		{
			// The themes directory doesn't exist at all. no need to delete it.
			return false;
		}

		// The directory exists but we don't know the version, better delete it to be safe.
		fsDirClose(&dir);
		return true;
	}

	// The file exists, read the content and check the version.	
	char disk_version[sizeof(fw.version_hash)] = {0};
	u64 bytes_read = 0;
	Result rc = fsFileRead(&versionFile, 0, disk_version, sizeof(disk_version), 0, &bytes_read);

	fsFileClose(&versionFile);

	if (R_FAILED(rc))
		return true;

	if (bytes_read != sizeof(disk_version))
		return true;

	if (!memcmp(disk_version, fw.version_hash, sizeof(disk_version)))
		return false;

	return true;
}

int main(int argc, char* argv[])
{
	// Some bad sd cards are supid slow to boot. Even 2 minutes.
	// https://discord.com/channels/643436008452521984/643456136887926813/1527002400588566528
	// Use 5 minutes as timeout so we're sure to catch all cases. If a sd card takes that long it's beyond salvation.
	Result rc = eventWait(&homeMenuLaunched, 300E+9);
	u64 pid = 0;

	if (R_SUCCEEDED(rc))
	{
		// We prevented the home menu from launching!
		// Check the system version, clean up if needed, then resume boot.
		if (should_delete_themes())
			fsFsDeleteDirectoryRecursively(&sdCard, "/atmosphere/contents/0100000000001000");
		
		// Resume the home menu
		rc = pmdmntGetProcessId(&pid, QLAUNCH_ID);
		if (R_FAILED(rc)) fatalThrow(rc);

		rc = pmdmntStartProcess(pid);
		if (R_FAILED(rc)) fatalThrow(rc);
	}
	else if (rc == KERNELRESULT(TimedOut))
	{
		// This branch should never happen because our hook should always trigger before qlaunch.
		// In case it does, call fatal for now. We are interested in knowing if our impl suffers from race conditions.
		// Not calling fatal here would allow the user to boot with a potentially incompatible theme which would crash qlaunch.
		// Instead, we want to know early if your sysmodule failed so we should be the ones to crash the console.
		if (R_SUCCEEDED(pmdmntGetProcessId(&pid, QLAUNCH_ID)) && pid)
		{
			// We were too slow?
			fatalThrow(MAKERESULT(Module_Libnx, LibnxError_HandleTooEarly));
		}
		else
		{
			// Qlaunch is not running at all?
			// Previously we would crash here but let's avoid all the support cases that come from this debug assert.
			// fatalThrow(MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen));
		}
	}
	else
	{
		fatalThrow(rc);
	}
	
	// Cleanly exit to free sysmodule memory
	return 0;
}
