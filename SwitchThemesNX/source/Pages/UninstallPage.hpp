#pragma once
#include "../UI/UI.hpp"

class UninstallPage : public IPage
{
	public:
		UninstallPage();	
		
		void Render(int X, int Y) override;
		void Update() override;
};