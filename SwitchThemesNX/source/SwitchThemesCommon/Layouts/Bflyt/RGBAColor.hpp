#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "../../MyTypes.h"
#include <iomanip>
#include "../../BinaryReadWrite/Buffer.hpp"

class RGBAColor
{
public:
	u8 R = 0, G = 0, B = 0, A = 0;

	RGBAColor() {};
	RGBAColor(u8 r, u8 g, u8 b, u8 a = 255) : R(r), G(g), B(b), A(a) {}
	RGBAColor(const std::string& LeByteString)
	{
		u32 Col = std::stoull(LeByteString, nullptr, 16);
		R = (Col & 0xFF);
		G = ((Col >> 8) & 0xFF);
		B = ((Col >> 16) & 0xFF);
		A = ((Col >> 24) & 0xFF);
	}
	
	//colors are encoded as 0xAABBGGRR
	std::string AsString() 
	{
		std::stringstream str;
		str << std::hex << std::setw(8) << ((u32)(R | G << 8 | B << 16 | A << 24));
		return str.str();
	}

	static RGBAColor Read(Buffer& buf) 
	{
		RGBAColor col;
		col.R = buf.readUInt8();
		col.G = buf.readUInt8();
		col.B = buf.readUInt8();
		col.A = buf.readUInt8();
		return col;
	}

	void Write(Buffer& buf)
	{
		buf.Write(R);
		buf.Write(G);
		buf.Write(B);
		buf.Write(A);
	}
};