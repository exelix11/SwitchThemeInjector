namespace BnTxx.Formats
{
    public enum TextureFormatType
    {
        R5G6B5    = 0x07,
        R8G8      = 0x09,
        R16       = 0x0a,
        R8G8B8A8  = 0x0b,
        R11G11B10 = 0x0f,
        R32       = 0x14,
        BC1       = 0x1a,
        BC2       = 0x1b,
        BC3       = 0x1c,
        BC4       = 0x1d,
        BC5       = 0x1e,
        ASTC4x4   = 0x2d,
        ASTC5x4   = 0x2e,
        ASTC5x5   = 0x2f,
        ASTC6x5   = 0x30,
        ASTC6x6   = 0x31,
        ASTC8x5   = 0x32,
        ASTC8x6   = 0x33,
        ASTC8x8   = 0x34,
        ASTC10x5  = 0x35,
        ASTC10x6  = 0x36,
        ASTC10x8  = 0x37,
        ASTC10x10 = 0x38,
        ASTC12x10 = 0x39,
        ASTC12x12 = 0x3a
    }

	public enum TextureFormatVar
	{
		UNorm = 1,
		SNorm = 2,
		UInt = 3,
		SInt = 4,
		Single = 5,
		SRGB = 6,
		UHalf = 10
	}

	public enum TextureType
	{
		Image1D = 0,
		Image2D = 1,
		Image3D = 2,
		Cube = 3,
		CubeFar = 8
	}

	public enum ChannelType
	{
		Zero,
		One,
		Red,
		Green,
		Blue,
		Alpha
	}
}
