#pragma once
#include <vector>
#include <string>
#include <tuple>
#include "../MyTypes.h"
#include "../BinaryReadWrite/Buffer.hpp"
#include "BRTI.hpp"
#include "DDS.hpp"

class QuickBntx 
{
public:
	std::vector<Bntxx::BRTI> Textures;
	std::vector<u8> Rlt;

	QuickBntx(Buffer &Reader);

	std::vector<u8> Write();
	void ReplaceTex(const string &name, const DDSEncoder::DDSLoadResult &tex);
	Bntxx::BRTI* FindTex(const string &name);
private:
	std::vector<u8> Head;
};