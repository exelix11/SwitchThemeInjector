#pragma once
#include <iostream>
#include <string>
#include "UI/DialogPages.hpp"

const std::string VersionString = "Ver. 1.5";
extern std::string SystemVer;

extern bool UseAnimations;

void PushPage(IUIControlObj* page);
void PushPageBlocking(IUIControlObj* page);
void PopPage();
void ErrorFatal(const std::string &msg);
void Dialog(const std::string &msg);
void DialogBlocking(const std::string &msg);
void DisplayLoading(const std::string &msg);
void QuitApp();