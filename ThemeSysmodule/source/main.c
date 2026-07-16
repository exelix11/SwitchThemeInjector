#include <switch.h>
#include <string.h>

#define QLAUNCH_ID 0x0100000000001000

static FsFileSystem sdCard;
static SetSysFirmwareVersion fw;

// This version tag is used by the theme installer to detect the version of the sysmodule from the binary
volatile char version_tag[] = "THEME_SYMODULE:2=";

static void sd_open() 
{
	// We use this directly because we don't want to depend on fsdev which requires malloc
	Result rc = fsOpenSdCardFileSystem(&sdCard);
	if (R_FAILED(rc))
		fatalThrow(rc);
}

static void init_cur_version() 
{
	Result rc = setsysInitialize();
	if (R_SUCCEEDED(rc)) {
		rc = setsysGetFirmwareVersion(&fw);
		if (R_FAILED(rc)) fatalThrow(rc);

		hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
		setsysExit();
	}

	// Should not be needed, but just in case.
	fw.version_hash[sizeof(fw.version_hash) - 1] = 0;
}

#define VERSION_FILE "version_hash.bin"
#define VERSION_BACKUP_FILE "old_version_hash.bin"

static void process_qlaunch();

#if !defined(NRO_BUILD)
#pragma message "SYSMODULE BUILD"
#define QLAUNCH_ROOT "/atmosphere/contents/0100000000001000/"

static Event homeMenuLaunched;

// We try to have the smallest impact on memory as possible.
// By default, we don't use the heap at all.
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

#define debug(...) do {} while(0)

u32 __nx_applet_type = AppletType_None;
u32 __nx_fs_num_sessions = 1;
u32 __nx_fsdev_direntry_cache_size = 1;

void __attribute__((weak)) __appInit(void)
{
	Result rc;

	rc = smInitialize();
	if (R_FAILED(rc))
		fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

	// pmdmnt was changed with fw updates, libnx needs to know hos version to call the correct functions.
	init_cur_version();

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

	sd_open();
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
int main(int argc, char* argv[])
{
	// Some bad sd cards are supid slow to boot. Even 2 minutes.
	// https://discord.com/channels/643436008452521984/643456136887926813/1527002400588566528
	// Use 5 minutes as timeout so we're sure to catch all cases. If a sd card takes that long it's beyond salvation.
	Result rc = eventWait(&homeMenuLaunched, SECONDS(300));
	u64 pid = 0;

	if (R_SUCCEEDED(rc))
	{
		process_qlaunch();

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
#else

#include <stdio.h>

#pragma message "NRO BUILD"
#define QLAUNCH_ROOT "/test_atmosphere/contents/0100000000001000/"

#define debug(...) printf(__VA_ARGS__)

// The NRO_BUILD is a test version of the sysmodule that can be built as a normal homebrew to test file operations. This will not hook the home menu at all.
int main(int argc, char* argv[])
{
	consoleInit(NULL);
	PadState pad;
	padInitializeDefault(&pad);
	
	init_cur_version();
		
	// Dev: simulate a different firmware manually
	// fw.version_hash[0] = 'l';

	debug("Current firmware hash: %s\n", fw.version_hash);

	sd_open();
	process_qlaunch();
	fsFsClose(&sdCard);

	printf("Press + to exit\n");

	while (appletMainLoop())
	{
		padUpdate(&pad);
		u64 kDown = padGetButtonsDown(&pad);

		if (kDown & HidNpadButton_Plus)
			break; // break in order to return to hbmenu

		consoleUpdate(NULL);
	}

	consoleExit(NULL);
	return 0;
}
#endif

static void remove_last_path_piece(char* str)
{
	char* end = str + strlen(str) - 1;
	while (end >= str)
	{
		if (*end == '/')
		{
			*end = 0;
			return;
		}

		end--;
	}
}

static void path_join(char* destination, int size, const char* name)
{
	if (*destination && destination[strlen(destination) - 1] != '/')
		strncat(destination, "/", size - 1);

	strncat(destination, name, size - 1);
}

static void path_join_to(char* destination, int size, const char* root, const char* name)
{
	*destination = 0;
	strncat(destination, root, size - 1);
	path_join(destination, size, name);
}

typedef struct {
	char hash[0x40];
} VersionHash;

_Static_assert(sizeof(VersionHash) == sizeof(fw.version_hash), "Version hash size mismatch");

static bool read_version_hash(FsFile* file, VersionHash* current_hash)
{
	u64 bytes_read = 0;
	memset(current_hash, 0, sizeof(*current_hash));
	Result rc = fsFileRead(file, 0, current_hash->hash, sizeof(current_hash->hash), 0, &bytes_read);
	fsFileClose(file);

	// Ensure that the file terminates with a 0 byte
	current_hash->hash[sizeof(current_hash->hash) - 1] = 0;

	if (R_FAILED(rc))
	{
		debug("Failed to read version hash file: 0x%x\n", rc);
		return false;
	}

	if (bytes_read != sizeof(current_hash->hash))
	{
		debug("Version hash file is the wrong size: %lu\n", bytes_read);
		return false;
	}

	return true;
}

typedef enum {
	VersionCheck_Failure, // A failure causes us to delete the folder entirely to prevent corruption issues
	VersionCheck_DoNothing, // Flag that immediately terminates.
	VersionCheck_NoThemes, // Themes are not present, check if we can restore a previous version
	VersionCheck_ThemeMismatch // Currently installed themes are
} VersionCheckResult;

static bool is_hash_empty(VersionHash* hash)
{
	for (int i = 0; i < sizeof(hash->hash); i++)
	{
		if (hash->hash[i] != 0)
			return false;
	}

	return true;
}

static VersionCheckResult version_check(VersionHash* current_hash)
{
	FsFile versionFile;

	if (R_FAILED(fsFsOpenFile(&sdCard, QLAUNCH_ROOT VERSION_FILE, FsOpenMode_Read, &versionFile)))
	{
		FsDir dir;
		if (R_FAILED(fsFsOpenDirectory(&sdCard, QLAUNCH_ROOT "romfs", FsDirOpenMode_ReadFiles | FsDirOpenMode_NoFileSize, &dir)))
		{
			// The themes directory doesn't exist at all. no need to delete it.
			debug("romfs directory doesn't exist\n");
			return VersionCheck_NoThemes;
		}

		// The directory exists but we don't know the version, better delete it to be safe.
		debug("romfs directory exists but without version info\n");
		fsDirClose(&dir);
		return VersionCheck_Failure;
	}

	// The file exists, read the content and check the version.	
	if (!read_version_hash(&versionFile, current_hash))
	{
		debug("Failed to read version hash file\n");
		return VersionCheck_Failure;
	}

	if (is_hash_empty(current_hash))
	{
		debug("Version hash file is empty\n");
		return VersionCheck_Failure;
	}

	if (!memcmp(current_hash->hash, fw.version_hash, sizeof(current_hash->hash)))
	{
		debug("Version hash matches\n");
		return VersionCheck_DoNothing;
	}

	debug("Version hash mismatch\n");
	return VersionCheck_ThemeMismatch;
}

static bool backup_romfs(VersionHash* installed_hash)
{
	char new_path[FS_MAX_PATH] = { 0 };
	strcat(new_path, QLAUNCH_ROOT "bak_");
	strcat(new_path, installed_hash->hash); // We know empirically that this is a printable hash

	debug("Backing up romfs to %s\n", new_path);

	FsDir tmp;
	if (R_SUCCEEDED(fsFsOpenDirectory(&sdCard, new_path, FsDirOpenMode_ReadFiles, &tmp)))
	{
		debug("Backup directory already exists, deleting it\n");
		fsDirClose(&tmp);
		if (R_FAILED(fsFsDeleteDirectoryRecursively(&sdCard, new_path)))
			return false;
	}
	
	Result rc = fsFsRenameDirectory(&sdCard, QLAUNCH_ROOT "romfs", new_path);
	if (R_FAILED(rc))
	{
		debug("Failed to rename romfs directory to %s: 0x%x\n", new_path, rc);
		return false;
	}

	path_join(new_path, sizeof(new_path), VERSION_BACKUP_FILE);
	rc = fsFsRenameFile(&sdCard, QLAUNCH_ROOT VERSION_FILE, new_path);
	if (R_FAILED(rc))
	{
		debug("Failed to rename version file to %s: 0x%x\n", new_path, rc);
		return false;
	}

	return true;
}

#define ITERATE_CONTINUE 0
#define ITERATE_STOP MAKERESULT(Module_Libnx, LibnxError_ShouldNotHappen)

typedef Result (*IterCallback)(s64 index, const char* root, FsDirectoryEntry* entry);

static Result iterate_directories(const char* path, FsDirOpenMode mode, IterCallback callback)
{
	FsDir dir;
	Result rc = fsFsOpenDirectory(&sdCard, path, mode, &dir);
	if (R_FAILED(rc))
	{
		debug("Failed to open %s with mode %d: 0x%x\n", path, mode, rc);
		return rc;
	}

	s64 count = 0;
	rc = fsDirGetEntryCount(&dir, &count);
	if (R_FAILED(rc))
	{
		debug("Failed to get entry count for %s with mode %d: 0x%x\n", path, mode, rc);
		fsDirClose(&dir);
		return rc;
	}

	debug("Opened %s with mode %d. %ld entries\n", path, mode, count);

	for (s64 i = 0; i < count; i++)
	{
		FsDirectoryEntry entry;
		s64 read_count = 0;
		rc = fsDirRead(&dir, &read_count, 1, &entry);
		if (R_FAILED(rc))
		{
			debug("Failed to read entry %ld: 0x%x\n", i, rc);
			break;
		}

		if (!read_count)
		{
			debug("Enumeration done\n");
			break;
		}

		debug("Found entry %d %s\n", entry.type, entry.name);

		// Ensure we don't accidentally read a path that is too long and gets cut off by path handling code
		// Path buffer is 0x300 so this is a reasonably safe limit given all the other paths are statically known.
		if (strlen(entry.name) > 0x100)
		{
			debug("Skipping %s because the name is too long\n", entry.name);
			continue;
		}
		
		rc = callback(i, path, &entry);
		if (rc == ITERATE_STOP)
		{
			debug("Callback requested stop\n");
			rc = 0;
			break;
		}
		else if (R_FAILED(rc))
		{
			debug("Callback failed: 0x%x\n", rc);
			break;
		}
	}

	debug("Done iterating %s %x\n", path, rc);

	fsDirClose(&dir);
	return rc;
}

static Result restore_callback(s64 index, const char* root, FsDirectoryEntry* entry)
{
	char tmp_path[FS_MAX_PATH] = { 0 };
	path_join_to(tmp_path, sizeof(tmp_path), root, entry->name);
	path_join(tmp_path, sizeof(tmp_path), VERSION_BACKUP_FILE);

	FsFile file;
	Result rc = fsFsOpenFile(&sdCard, tmp_path, FsOpenMode_Read, &file);
	if (R_FAILED(rc))
	{
		debug("backup missing %s: 0x%x\n", tmp_path, rc);
		return ITERATE_CONTINUE;
	}

	VersionHash hash = { 0 };
	if (read_version_hash(&file, &hash) && !memcmp(hash.hash, fw.version_hash, sizeof(hash.hash)))
	{
		debug("RESTORING %s\n", tmp_path);

		rc = fsFsRenameFile(&sdCard, tmp_path, QLAUNCH_ROOT VERSION_FILE);
		if (R_FAILED(rc))
		{
			debug("Failed to rename %s to version file: 0x%x\n", tmp_path, rc);
			return rc;
		}

		remove_last_path_piece(tmp_path);
		rc = fsFsRenameDirectory(&sdCard, tmp_path, QLAUNCH_ROOT "romfs");
		if (R_FAILED(rc))
		{
			debug("Failed to rename %s to romfs: 0x%x\n", tmp_path, rc);
			return rc;
		}

		return ITERATE_STOP;
	}

	return ITERATE_CONTINUE;
}

static bool restore_current()
{
	Result rc = iterate_directories(QLAUNCH_ROOT, FsDirOpenMode_ReadDirs, restore_callback);
	if (R_FAILED(rc))
	{
		debug("Failed to iterate restore_current: 0x%x\n", rc);
		return false;
	}
	
	return true;
}

static FsTimeStampRaw oldest_time;
static FsDirectoryEntry oldest_entry;
static int oldest_count;

static Result remove_old_callback(s64 index, const char* root, FsDirectoryEntry* entry)
{
	char tmp_path[FS_MAX_PATH] = { 0 };
	path_join_to(tmp_path, sizeof(tmp_path), root, entry->name);
	path_join(tmp_path, sizeof(tmp_path), VERSION_BACKUP_FILE);

	FsTimeStampRaw tmp_time = { 0 };
	FsFile file;
	Result rc = fsFsOpenFile(&sdCard, tmp_path, FsOpenMode_Read, &file);
	if (R_FAILED(rc))
	{
		// It's ok, the file might not exist if this is not a backup folder
		debug("Failed to open %s: 0x%x\n", tmp_path, rc);
		return ITERATE_CONTINUE;
	}

	rc = fsFsGetFileTimeStampRaw(&sdCard, tmp_path, &tmp_time);
	if (R_FAILED(rc))
	{
		debug("Failed to get timestamp for %s: 0x%x\n", tmp_path, rc);
		fsFileClose(&file);
		return rc;
	}
	
	fsFileClose(&file);
	debug("Timestamp for %s: valid=%d created=%lu modified=%lu\n", tmp_path, tmp_time.is_valid, tmp_time.created, tmp_time.modified);

	if (!tmp_time.is_valid)
		return ITERATE_CONTINUE;

	++oldest_count; 
	if (!oldest_time.is_valid || tmp_time.modified < oldest_time.modified)
	{
		oldest_time = tmp_time;
		oldest_entry = *entry;
	}

	return ITERATE_CONTINUE;
}

static bool remove_old()
{
	memset(&oldest_time, 0, sizeof(oldest_time));
	memset(&oldest_entry, 0, sizeof(oldest_entry));
	oldest_count = 0;

	Result rc = iterate_directories(QLAUNCH_ROOT, FsDirOpenMode_ReadDirs, remove_old_callback);
	if (R_FAILED(rc))
	{
		debug("Failed to iterate restore_current: 0x%x\n", rc);
		return false;
	}

	debug("Found %d valid old version files, oldest is %s\n", oldest_count, oldest_count ? oldest_entry.name : "(none)");

	if (oldest_count > 3)
	{
		char tmp_path[FS_MAX_PATH] = { 0 };
		path_join_to(tmp_path, sizeof(tmp_path), QLAUNCH_ROOT, oldest_entry.name);

		debug("REMOVING oldest %s\n", tmp_path);
		rc = fsFsDeleteDirectoryRecursively(&sdCard, tmp_path);
		if (R_FAILED(rc))
		{
			debug("Failed to delete %s: 0x%x\n", tmp_path, rc);
			return false;
		}
	}

	return true;
}

static void process_qlaunch()
{
	// We prevented the home menu from launching
	VersionHash installed = { 0 };
	switch (version_check(&installed)) 
	{
		case VersionCheck_DoNothing:
			debug("Version check passed, doing nothing\n");
			return;
		case VersionCheck_Failure:
			debug("Version check failure.\n");
		handle_failure:
			debug("Deleting QLAUNCH_ROOT\n\n");
			fsFsDeleteDirectoryRecursively(&sdCard, QLAUNCH_ROOT);
			return;
		case VersionCheck_NoThemes: {
			debug("Version reports no themes.\n");
			// If no themes are detected, just try to restore a previous version in case it exists
			if (!restore_current())
			{
				debug("Failed to restore current version\n");
				goto handle_failure;
			}
			return;
		}
		case VersionCheck_ThemeMismatch: {
			debug("Version reports theme version mismatch.\n");
			// If we're on a different firmware version, first backup the current version.
			if (!backup_romfs(&installed))
			{
				debug("Failed to backup romfs\n");
				goto handle_failure;
			}
			// Then try to restore the last known good version.
			if (!restore_current())
			{
				debug("Failed to restore current version\n");
				goto handle_failure;
			}
			// Finally, avoid to fill up the sd card with old versions and remove the oldest version if we have too many.
			if (!remove_old())
			{
				debug("Remove old version failed\n");
				goto handle_failure;
			}
			return;
		}
	}
}