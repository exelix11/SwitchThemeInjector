#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include "../../BinaryReadWrite/Buffer.hpp"
#include "../../MyTypes.h"
#include "../Patches.hpp"
#include <memory>
#include "BasePane.hpp"
#include "BflytMaterial.hpp"
#include <queue>
#include <functional>

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
	//Maybe make this stl-comatible ?
	class Iterator : public std::iterator<std::forward_iterator_tag,Panes::PanePtr, Panes::PanePtr, const Panes::PanePtr*, Panes::PanePtr&>
	{
	public:
		Iterator(BflytFile *file, std::vector<Panes::PanePtr>&& root) : _file(file) 
		{
			for (auto& p : root)
				it.push(p);
			Step();
		}

		Iterator(BflytFile *file, std::vector<Panes::PanePtr>& root) : _file(file)
		{
			for (auto& p : root)
				it.push(p);
			Step();
		}

		const Panes::PanePtr& operator *() const
		{
			if (!Cur)
				throw std::runtime_error("The end of the sequence has been reached");
			return Cur;
		}

		Iterator& operator ++()	{ Step(); return *this; }
		bool operator==(const Iterator& other) const { return this->_file == other._file && this->Cur == other.Cur; }
		bool operator!=(const Iterator& other) const { return this->_file != other._file || this->Cur != other.Cur; }
	private:

		void Step()
		{
			if (it.size() == 0)
				Cur = nullptr;
			else {
				Cur = it.front();
				it.pop();
				for (auto ptr : Cur->Children)
					it.push(ptr);
			}
		}

		BflytFile *_file;
		std::queue<Panes::PanePtr> it;
		Panes::PanePtr Cur = nullptr;
	};

	Iterator PanesBegin() { return Iterator(this, RootPanes); }
	Iterator PanesEnd() { return Iterator(this, {}); }

	BflytFile(const std::vector<u8>& file);
	~BflytFile();
	std::vector<u8> SaveFile();

	u32 Version;

	std::vector<Panes::PanePtr> RootPanes;

	std::shared_ptr<Panes::TextureSection> GetTexSection();
	std::shared_ptr<Panes::MaterialsSection> GetMatSection();
	Panes::PanePtr GetRootElement();
	std::shared_ptr<Panes::Grp1Pane> GetRootGroup();

	Panes::PanePtr operator[] (const std::string &name);
	
	void RemovePane(Panes::PanePtr& pane);
	void AddPane(size_t offset, Panes::PanePtr& Pane, Panes::PanePtr& pane);
	void MovePane(Panes::PanePtr& pane, Panes::PanePtr& NewParent, size_t offset);

	Panes::PanePtr FindPane(std::function<bool(const Panes::PanePtr&)> fun);
	Panes::PanePtr FindRoot(const std::string& type);
private:
	int FindRootIndex(const std::string& type);
	std::vector<Panes::PanePtr> WritePaneListForBinary();
};