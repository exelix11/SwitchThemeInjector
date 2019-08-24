#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../../BinaryReadWrite/Buffer.hpp"
#include "../../MyTypes.h"
#include "../Patches.hpp"
#include <memory>
#include "BasePane.hpp"
#include "BflytMaterial.hpp"

namespace Panes
{
	class Grp1Pane;

	class TextureSection : public BasePane
	{
	public:
		std::vector<std::string> Textures;
		TextureSection(Buffer& reader);
		TextureSection();

		void ApplyChanges(Buffer& writer) override;
	};

	class MaterialsSection : public BasePane
	{
	public:
		u32 Version;

		std::vector<BflytMaterial> Materials;
		MaterialsSection(Buffer& reader, u32 version);
		MaterialsSection(u32 version);

		void ApplyChanges(Buffer& writer) override;
	};

	typedef std::shared_ptr<Panes::BasePane> PanePtr;
}

namespace Utils 
{
	template <typename T>
	static inline size_t IndexOf(const std::vector<T>& v, const T& s)
	{
		for (size_t i = 0; i < v.size(); i++)
			if (v[i] == s) return i;
		return SIZE_MAX;
	}
}

class BflytFile 
{
public:
	BflytFile(const std::vector<u8>& file);
	~BflytFile();
	std::vector<u8> SaveFile();

	u32 Version;

	std::shared_ptr<Panes::TextureSection> GetTexSection();
	std::shared_ptr<Panes::MaterialsSection> GetMatSection();

	Panes::PanePtr& operator[] (int index);
	std::vector<Panes::PanePtr> Panes;
	int FindPaneEnd(int paneIndex);

	std::vector<std::string> GetPaneNames();
	std::vector<std::string> GetGroupNames();
	
	Panes::PanePtr RootPane = nullptr;
	std::shared_ptr<Panes::Grp1Pane> RootGroup = nullptr;

	void RemovePane(Panes::PanePtr& pane);
	void AddPane(size_t offsetInChildren, Panes::PanePtr& Pane, std::vector<Panes::PanePtr>& pane);
	void MovePane(Panes::PanePtr& pane, Panes::PanePtr& NewParent, size_t childOffset);

	void RebuildParentingData();
private:
	void RebuildGroupingData();
};