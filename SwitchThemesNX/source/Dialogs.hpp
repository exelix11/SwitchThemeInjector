#pragma once
#include <iostream>
#include <functional>
#include <string>

void Dialog(const std::string &msg);

//executes a function after the drawing loop is terminated
void PushFunction(const std::function<void()>& fun);

//These functions can only be called during the update function as they need to draw outside of the main loop
void DialogBlocking(const std::string &msg);
void DisplayLoading(const std::string &msg);
void DisplayLoading(std::initializer_list<std::string> lines);