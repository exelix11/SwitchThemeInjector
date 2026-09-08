#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <format>

#include "UI/UIManagement.hpp"
#include "UI/UI.hpp"
#include "Platform/Platform.hpp"

#include "Pages/ThemePage.hpp"
#include "Pages/CfwSelectPage.hpp"
#include "Pages/UninstallPage.hpp"
#include "Pages/NcaDumpPage.hpp"
#include "Pages/TextPage.hpp"
#include "Pages/ExternalInstallPage.hpp"
#include "ViewFunctions.hpp"
#include "SwitchThemesCommon/Common.hpp"
#include "Pages/RemoteInstallPage.hpp"
#include "Pages/SettingsPage.hpp"
#include "Pages/RebootPage.cpp"
#include "Pages/QlaunchPatchPage.hpp"

#include "SwitchTools/PatchMng.hpp"
#include "fs.hpp"

const int DebugOverlay = false;

static inline void ImguiBindController()
{
	ImGuiIO& io = ImGui::GetIO();

	NAV_DOWN	= (KeyPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)	|| StickAsButton(1) > .3f	|| StickAsButton(3) > .3f );
	NAV_UP		= (KeyPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP)		|| StickAsButton(1) < -.3f	|| StickAsButton(3) < -.3f);
	NAV_LEFT	= (KeyPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT)	|| StickAsButton(0) < -.3f	|| StickAsButton(2) < -.3f);
	NAV_RIGHT	= (KeyPressed(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT)	|| StickAsButton(0) > .3f	|| StickAsButton(2) > .3f );

	io.NavInputs[ImGuiNavInput_DpadDown]	= NAV_DOWN	? 1.0f : 0;
	io.NavInputs[ImGuiNavInput_DpadUp]		= NAV_UP	? 1.0f : 0;
	io.NavInputs[ImGuiNavInput_DpadLeft]	= NAV_LEFT	? 1.0f : 0;
	io.NavInputs[ImGuiNavInput_DpadRight]	= NAV_RIGHT	? 1.0f : 0;

	io.NavInputs[ImGuiNavInput_Activate] = KeyPressed(GLFW_GAMEPAD_BUTTON_A);
	io.NavInputs[ImGuiNavInput_Cancel] = KeyPressed(GLFW_GAMEPAD_BUTTON_B);

	io.NavActive = true;
	io.NavVisible = true;
}

static bool IsRendering = false;
static std::vector<IUIControlObj*> Pages;
static std::vector<IUIControlObj*> PopList;

static IUIControlObj *ViewObj = nullptr;

void PopPage(IUIControlObj* page)
{
	PopList.push_back(page);
}

static void _PopPage()
{
	auto ShouldRemove = [](IUIControlObj* obj) -> bool 
	{
		auto p = std::find(PopList.begin(), PopList.end(), obj);
		if (p == PopList.end()) return false;
		delete *p;
		return true;
	};

	Pages.erase(std::remove_if(Pages.begin(), Pages.end(), ShouldRemove));
	PopList.clear();

	ViewObj = Pages.size() ? Pages.back() : nullptr;
}

//All pages must be dynamically allocated
void PushPage(IUIControlObj* page)
{
	Pages.push_back(page);
	ViewObj = page;
}

std::vector<std::function<void()>> DeferredFunctions;
void PushFunction(const std::function<void()>& fun)
{
	DeferredFunctions.push_back(fun);
}

static void ExecuteDeferredFunctions() 
{
	auto vec = std::move(DeferredFunctions);
	DeferredFunctions = {};
	for (auto& fun : vec)
		fun();
}

void Dialog(const std::string &msg)
{
	PushPage(new DialogPage(msg));
}

static inline void DisplayLoading(LoadingOverlay &&o)
{
	GFX::StartFrame();
	o.Render(0, 0);
	GFX::EndFrame();
}

void DisplayLoading(const std::string& msg)
{
	DisplayLoading(LoadingOverlay(msg));
}

void DisplayLoading(std::initializer_list<std::string> lines)
{
	DisplayLoading(LoadingOverlay(lines));
}

void FatalError(std::string message)
{
	LOGf("Fatal error: %s\n", message.c_str());
	DialogBlocking("Fatal error: " + message + "\n\nThe application will close");
	exit(-1);
}

static void RenderDebugOverlay()
{
	static double previousTime = glfwGetTime();
	static int frameCount = 0;
	static int fpsValue = 0;

	auto dlist = ImGui::GetForegroundDrawList();

	double currentTime = glfwGetTime();
	frameCount++;

	if (currentTime - previousTime >= 1.0)
	{
		fpsValue = frameCount;

		frameCount = 0;
		previousTime = currentTime;
	}

	auto text = std::format("FPS: {}", fpsValue);
	dlist->AddText(ImVec2{0, 0}, 0xffffffff, text.c_str());

	text = std::format("Pages: {} PopList: {}", Pages.size(), PopList.size());
	dlist->AddText(ImVec2{0, 20}, 0xffffffff, text.c_str());
	
	text = std::format("Images: {}", RenderImage::DebugLoadedImages());
	dlist->AddText(ImVec2{0, 40}, 0xffffffff, text.c_str());

	size_t cacheSize;
	int cacheCount;
	float cachePercent;

	ImageCache::DebugInformation(cacheSize, cacheCount, cachePercent);

	text = std::format("Cache: {} bytes ({} images, {:.2f}%)", cacheSize, cacheCount, cachePercent);
	dlist->AddText(ImVec2{ 0, 60 }, 0xffffffff, text.c_str());
}

static inline void MainLoop(bool BreakOnPop = false)
{
	while (App::MainLoop() && ViewObj)
	{
		PlatformGetInputs();
		ImguiBindController();
		PlatformImguiBinds();

		//A control may push a page either in the render or the update function.
		IUIControlObj* CurObj = ViewObj;

		IsRendering = true;
		GFX::StartFrame();

		try 
		{
			CurObj->Render(0, 0);
		}
		catch (std::exception& ex)
		{
			// This will likely still fail due to imgui throwing on end frame if the page was not properly popped
			std::string msg = ex.what();
			PushFunction([msg] {
				FatalError(msg);
			});
		}

		if (DebugOverlay)
			RenderDebugOverlay();

		GFX::EndFrame();
		IsRendering = false;

		try {
			if (CurObj == ViewObj)
				CurObj->Update();

			ExecuteDeferredFunctions();
		}
		catch (std::exception& ex)
		{
			FatalError(ex.what());
		}

		if (PopList.size())
		{
			_PopPage();
			if (BreakOnPop)
				break;
		}

		PlatformSleep(1 / 30.0f * 1000);
	}
}

void PushPageBlocking(IUIControlObj* page)
{
	if (IsRendering)
	{
		throw std::runtime_error("Attempted to push a blocking page while rendering");
	}

	PushPage(page);
	MainLoop(true);
}

void DialogBlocking(const std::string& msg)
{
	PushPageBlocking(new DialogPage(msg));
}

class QuitPage : public IPage
{
	public:
		QuitPage()
		{
			Name = "Quit";
		}	
		
		void Render(int X, int Y) override {}
		
		void Update() override
		{
			App::Quit();
		}
};

static void SetCfwFolder()
{
	auto f = fs::cfw::SearchFolders();
	if (f.size() != 1)
		// Previously this page would let the user select between multiple CFW folders, but now they're all deprecated.
		// This check can either be 0 or 1, in case ams is not installed the user is shown an error.
		PushPageBlocking(new CfwSelectPage(f));
	else
		fs::cfw::SetFolder(f[0]);
}

static std::vector<std::string> GetArgsInstallList(int argc, char** argv)
{
	if (argc <= 1) return {};

	std::vector<std::string> Args(argc - 1);
	for (size_t i = 0; i < Args.size(); i++)
		Args[i] = std::string(argv[i + 1]);

	//Appstore-style args
	if (StrStartsWith(Args[0], "installtheme="))
	{
		std::string key = "installtheme=";
		std::string pathss;
		std::vector<std::string> paths;
		for (auto argvs : Args)
		{
			auto pos = argvs.find(key);
			size_t index;
			while (true)
			{
				index = argvs.find("(_)");
				if (index == std::string::npos) break;
				argvs.replace(index, 3, " ");
			}
			if (pos != std::string::npos)
				pathss = argvs.substr(pos + 13);

			if (!pathss.empty())
			{
				std::string path;
				std::stringstream stream(pathss);
				while (getline(stream, path, ',')) {
					if (fs::Exists(path) && std::filesystem::is_regular_file(path))
						paths.push_back(path);
				}
			}
		}
		return paths;
	}
	else //File args from nx-hbloader
	{
		std::vector<std::string> res;

		for (auto& s : Args)
			if (fs::Exists(s) && std::filesystem::is_regular_file(s))
				res.push_back(move(s));

		return res;
	}
}	

static void SetupSysVer()
{
#if __SWITCH__
	setsysInitialize();
	SetSysFirmwareVersion firmware;

	static_assert(sizeof(firmware.version_hash) == sizeof(hos::VersionHash), "Version hash size mismatch");

	auto res = setsysGetFirmwareVersion(&firmware);
	if (R_FAILED(res))
	{
		setsysExit();
		DialogBlocking("Could not get sys ver res=" + std::to_string(res));
		return;
	}
	
	hos::Version = { firmware.major,firmware.minor,firmware.micro };
	memcpy(hos::VersionHash.data(), firmware.version_hash, sizeof(firmware.version_hash));

	setsysExit();
#else 
	hos::Version = { 20,0,0 };
	memcpy(hos::VersionHash.data(), "fakever", sizeof("fakever"));
#endif
}

int main(int argc, char **argv)
{
	PlatformInit();

	if (!GFX::Init())
	{
		PlatformExit();
		return -1;
	}
	PlatformAfterInit();

	SetupSysVer();
	DisplayLoading("Loading system info...");

	fs::EnsureThemesFolderExists();
	SetCfwFolder();

	PatchMng::Init();

	QlaunchPatchPage* patchPage = new QlaunchPatchPage();
	bool missingPatches = patchPage->ShouldShow();

	if (envHasArgv() && argc > 1)
	{
		if (missingPatches)
		{
			// We don't have rendering here so at most we can show a yes/no dialog.
			if (YesNoPage::Ask(
				"You are missing home menu patches needed for lock screen themes. If you install a theme that is not compatibile it will cause your console to crash on boot.\n\n"
				"The theme installer can download these patches automatically.\nDo you want to open the download page?\n\n",
				"Cancel installation and open patch settings",
				"Continue installing anyway (not recommended)"
			))
			{
				goto SKIP_ARGUMENTS;
			}
		}

		auto paths = GetArgsInstallList(argc,argv);
		if (paths.size() != 0)
		{
			PushPage(new ExternalInstallPage(paths));
			MainLoop();
		}
	}	
	else
	{
		SKIP_ARGUMENTS:
		TabRenderer *t = new TabRenderer();
		PushPage(t);

		// Internally calls patchmng
		if (missingPatches)
			t->AddPage(patchPage);

		ThemesPage *p = new ThemesPage();
		t->AddPage(p);
		RemoteInstallPage* rmi = new RemoteInstallPage();
		t->AddPage(rmi);
		UninstallPage *up = new UninstallPage();
		t->AddPage(up);
		NcaDumpPage *dp = new NcaDumpPage();
		t->AddPage(dp);
		SettingsPage *sf = new SettingsPage();
		t->AddPage(sf);
		CreditsPage *credits = new CreditsPage();
		t->AddPage(credits);
		RebootPage *reboot = new RebootPage();
		t->AddPage(reboot);
		QuitPage *q = new QuitPage();
		t->AddPage(q);

		MainLoop();
		
		delete p;
		delete up;
		delete dp;
		delete rmi;
		delete sf;
		delete credits;
		delete q;
	}

	delete patchPage;

	while (Pages.size() != 0)
	{
		delete Pages.back();
		Pages.pop_back();
	}

	GFX::Exit();
	RenderImage::DebugAssertLeaks();
	PlatformExit();
	
    return 0;
}