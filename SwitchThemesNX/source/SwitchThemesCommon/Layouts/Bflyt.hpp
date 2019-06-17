#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../BinaryReadWrite/Buffer.hpp"
#include "../MyTypes.h"
#include "Patches.hpp"

namespace Panes 
{
	//Note: the u32 colors are encoded as 0xAABBGGRR

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

	class Grp1Pane : public BasePane 
	{
	public:
		u32 Version;
		std::string GroupName;
		std::vector<std::string> Panes;

		Grp1Pane(Buffer& buf, u32 version);
		Grp1Pane(u32 version);
		void WritePane(Buffer& writer) override;
	private:
		void LoadProperties();
		void ApplyChanges();
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

	class BflytMaterial 
	{
	public:
		//TODO: support more properties in the layouts (?)
		std::string Name;
		u32 ForegroundColor;
		u32 BackgroundColor;

		BflytMaterial(const std::vector<u8>& data, u32 Version, Endianness bo);
		std::vector<u8> Write(u32 version, Endianness bo);
	private:
		std::vector<u8> Data;
	};

	class MaterialsSection : BasePane
	{
	public:
		u32 Version;

		std::vector<BflytMaterial> Materials;
		MaterialsSection(Buffer &reader, u32 version);
		MaterialsSection(u32 version);

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

	class Usd1Pane : public BasePane
	{
	public:
		enum class ValueType : u8
		{
			data = 0,
			int32 = 1,
			single = 2,
			other = 3
		};

		struct EditableProperty
		{
			std::string Name;
			u64 ValueOffset;
			u16 ValueCount;
			ValueType type;

			std::vector<std::string> value;
		};

		std::vector<EditableProperty> Properties;
		//Discard the pointer if the Properties vector is changed
		EditableProperty *FindName(const std::string &name);	
		void AddProperty(const std::string &name, const std::vector<std::string> &values, ValueType type);

		Usd1Pane(Buffer &reader);
		void WritePane(Buffer &writer) override;
	private:
		std::vector<EditableProperty> AddedProperties;
		void LoadProperties();
		void ApplyChanges();
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
	bool PatchTextureName(const std::string &original, const std::string &_new);
	std::vector<std::string> GetPaneNames();
	std::vector<std::string> GetGroupNames();
	PatchResult ApplyLayoutPatch(const std::vector<PanePatch>& Patches);
	PatchResult ApplyMaterialsPatch(const std::vector<MaterialPatch>& Patches);
	PatchResult PatchBgLayout(const PatchTemplate& patch);
	PatchResult AddGroupNames(const std::vector<ExtraGroup>& Groups);
private:
	Panes::BasePane*& operator[] (int index);
	std::vector<Panes::BasePane*> Panes;
	PatchResult AddBgPanel(int index, const std::string &TexName, const std::string &Pic1Name);
	int AddBgMat(const std::string &texName);
};