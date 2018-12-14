#pragma once
#include <iostream>
#include <string>
#include "UI/DialogPages.hpp"

const std::string VersionString = "Beta 5";

void PushPage(IUIControlObj* page);
void PopPage();
void ErrorFatal(const std::string &msg);
void Dialog(const std::string &msg);
void DisplayLoading(const std::string &msg);
void QuitApp();