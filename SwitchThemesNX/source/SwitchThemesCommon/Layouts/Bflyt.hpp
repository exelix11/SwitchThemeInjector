#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../BinaryReadWrite/Buffer.hpp"
#include "../MyTypes.h"
#include "Patches.hpp"

namespace Panes 
{
	class BasePane
	{
	public:
		virtual std::string ToString();
		const std::string name;
		s32 length;
		std::vector<u8> data;

		BasePane(const std::string &_name, u32 len);
		BasePane(const BasePane &ref);
		BasePane(const std::string &_name, Buffer &reader);

		virtual void WritePane(Buffer &writer);
	};

	class PropertyEditablePane : public BasePane 
	{
	public :
		std::string ToString() override;
		std::string PaneName;
		Vector3 Position, Rotation;
		Vector2 Scale, Size;

		bool GetVisible();
		void SetVisible(bool);

		std::vector<u32> ColorData;

		PropertyEditablePane(const BasePane &p);
		void ApplyChanges();

		void WritePane(Buffer &writer) override;
	private:
		u8 _flag1;
	};

	class TextureSection : public BasePane
	{
	public:
		std::vector<std::string> Textures;
		TextureSection(Buffer &reader);
		TextureSection();

		void WritePane(Buffer &writer) override;
	};

	class MaterialsSection : BasePane
	{
	public:
		std::vector<std::vector<u8>> Materials;
		MaterialsSection(Buffer &reader);
		MaterialsSection();

		void WritePane(Buffer &writer) override;
	};

	class PicturePane : public BasePane
	{
	public:
		std::string ToString() override;
		std::string PaneName();

		PicturePane(Buffer &reader);
	private:
		std::string _PaneName;
	};
}

class BflytFile 
{
public:
	enum class PatchResult : u8 
	{
		AlreadyPatched,
		Fail,
		CorruptedFile,
		OK
	};

	BflytFile(const std::vector<u8>& file);
	~BflytFile();
	
	static std::string TryGetPanelName(const Panes::BasePane *p);

	u32 Version;

	Panes::TextureSection* GetTexSection();
	Panes::MaterialsSection* GetMatSection();

	std::vector<u8> SaveFile();
	void PatchTextureName(const std::string &original, const std::string &_new);
	std::vector<std::string> GetPaneNames();
	PatchResult ApplyLayoutPatch(const std::vector<PanePatch>& Patches);
	PatchResult PatchBgLayout(const PatchTemplate& patch);
private:
	Panes::BasePane*& operator[] (int index);
	std::vector<Panes::BasePane*> Panes;
	PatchResult AddBgPanel(int index, const std::string &TexName, const std::string &Pic1Name);
};