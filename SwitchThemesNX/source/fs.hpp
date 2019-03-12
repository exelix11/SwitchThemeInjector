#pragma once
#include "SwitchThemesCommon/MyTypes.h"
#include <switch.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <dirent.h>

extern std::string CfwFolder;
bool StrEndsWith(const std::string &str, const std::string &suffix);

std::vector<u8> OpenFile(const std::string &name);
void WriteFile(const std::string &name,const std::vector<u8> &data);

bool CheckThemesFolder();
std::vector<std::string> SearchCfwFolders();
std::vector<std::string> GetThemeFiles();

void UninstallTheme(bool full = false);
void CreateFsMitmStructure(const std::string &tid);
void CreateRomfsDir(const std::string &tid);
void CreateThemeStructure(const std::string &tid);

std::string GetFileName(const std::string &path);
std::string GetPath(const std::string &path);
std::string GetParentDir(const std::string &path);

void RecursiveDeleteFolder(const std::string &path);

std::string GetHomeNcaPath();
bool DumpHomeMenuNca();

void RemoveSystemDataDir();

int GetShuffleCount();
std::string MakeThemeShuffleDir();
void ClearThemeShuffle();

bool WriteHomeDumpVer();