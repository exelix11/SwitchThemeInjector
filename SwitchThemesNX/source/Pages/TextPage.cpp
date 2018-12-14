#include "TextPage.hpp"
#include "../input.hpp"

using namespace std;

TextPage::TextPage(const std::string &title, const std::string &text) : 
Text(text,WHITE, 900, font30)
{
	Name = title;
}

void TextPage::Render(int X, int Y)
{
	Text.Render(X + 20, Y + 20);
}

void TextPage::Update()
{	
	Parent->PageLeaveFocus(this);
}




