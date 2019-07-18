using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Collections;
namespace SwitchThemes.Common
{
	partial class ManagedYaz0
	{
		public static byte[] Compress(string FileName, int level = 3, int res1 = 0, int res2 = 0) => Compress(File.ReadAllBytes(FileName), level, res1, res2);
		public static byte[] Compress(byte[] Data, int level = 3, int reserved1 = 0, int reserved2 = 0)
		{
			int maxBackLevel = (int)(0x10e0 * (level / 9.0) - 0x0e0);
			if (maxBackLevel <= 0)
				maxBackLevel = 1;

			int dataptr = 0;

			byte[] result = new byte[Data.Length + Data.Length / 8 + 0x10];
			int resultptr = 0;
			result[resultptr++] = (byte)'Y';
			result[resultptr++] = (byte)'a';
			result[resultptr++] = (byte)'z';
			result[resultptr++] = (byte)'0';
			result[resultptr++] = (byte)((Data.Length >> 24) & 0xFF);
			result[resultptr++] = (byte)((Data.Length >> 16) & 0xFF);
			result[resultptr++] = (byte)((Data.Length >> 8) & 0xFF);
			result[resultptr++] = (byte)((Data.Length >> 0) & 0xFF);
			{
				var res1 = BitConverter.GetBytes(reserved1);
				var res2 = BitConverter.GetBytes(reserved2);
				if (BitConverter.IsLittleEndian)
				{
					Array.Reverse(res1);
					Array.Reverse(res2);
				}
				result[resultptr++] = (byte)res1[0];
				result[resultptr++] = (byte)res1[1];
				result[resultptr++] = (byte)res1[2];
				result[resultptr++] = (byte)res1[3];
				result[resultptr++] = (byte)res2[0];
				result[resultptr++] = (byte)res2[1];
				result[resultptr++] = (byte)res2[2];
				result[resultptr++] = (byte)res2[3];
			}
			int length = Data.Length;
			int dstoffs = 16;
			int Offs = 0;
			while (true)
			{
				int headeroffs = dstoffs++;
				resultptr++;
				byte header = 0;
				for (int i = 0; i < 8; i++)
				{
					int comp = 0;
					int back = 1;
					int nr = 2;
					{
						int ptr = dataptr - 1;
						int maxnum = 0x111;
						if (length - Offs < maxnum) maxnum = length - Offs;
						//Use a smaller amount of bytes back to decrease time
						int maxback = maxBackLevel;//0x1000;
						if (Offs < maxback) maxback = Offs;
						maxback = (int)dataptr - maxback;
						int tmpnr;
						while (maxback <= (int)ptr)
						{
							if (Data.Length - dataptr > 2 && Data[ptr] == Data[dataptr] && Data[ptr + 1] == Data[dataptr + 1] && Data[ptr+2] == Data[dataptr+2])
							{
								tmpnr = 3;
								while (tmpnr < maxnum && Data[ptr+tmpnr] == Data[dataptr+tmpnr]) tmpnr++;
								if (tmpnr > nr)
								{
									if (Offs + tmpnr > length)
									{
										nr = length - Offs;
										back = (int)(dataptr - ptr);
										break;
									}
									nr = tmpnr;
									back = (int)(dataptr - ptr);
									if (nr == maxnum) break;
								}
							}
							--ptr;
						}
					}
					if (nr > 2)
					{
						Offs += nr;
						dataptr += nr;
						if (nr >= 0x12)
						{
							result[resultptr++] = (byte)(((back - 1) >> 8) & 0xF);
							result[resultptr++] = (byte)((back - 1) & 0xFF);
							result[resultptr++] = (byte)((nr - 0x12) & 0xFF);
							dstoffs += 3;
						}
						else
						{
							result[resultptr++] = (byte)((((back - 1) >> 8) & 0xF) | (((nr - 2) & 0xF) << 4));
							result[resultptr++] = (byte)((back - 1) & 0xFF);
							dstoffs += 2;
						}
						comp = 1;
					}
					else
					{
						result[resultptr++] = Data[dataptr++];
						dstoffs++;
						Offs++;
					}
					header = (byte)((header << 1) | ((comp == 1) ? 0 : 1));
					if (Offs >= length)
					{
						header = (byte)(header << (7 - i));
						break;
					}
				}
				result[headeroffs] = header;
				if (Offs >= length) break;
			}
			while ((dstoffs % 4) != 0) dstoffs++;
			byte[] realresult = new byte[dstoffs];
			Array.Copy(result, realresult, dstoffs);
			return realresult;
		}

		public static byte[] Decompress(byte[] Data)
		{
			UInt32 leng = (uint)(Data[4] << 24 | Data[5] << 16 | Data[6] << 8 | Data[7]);
			byte[] Result = new byte[leng];
			int Offs = 16;
			int dstoffs = 0;
			while (true)
			{
				byte header = Data[Offs++];
				for (int i = 0; i < 8; i++)
				{
					if ((header & 0x80) != 0) Result[dstoffs++] = Data[Offs++];
					else
					{
						byte b = Data[Offs++];
						int offs = ((b & 0xF) << 8 | Data[Offs++]) + 1;
						int length = (b >> 4) + 2;
						if (length == 2) length = Data[Offs++] + 0x12;
						for (int j = 0; j < length; j++)
						{
							Result[dstoffs] = Result[dstoffs - offs];
							dstoffs++;
						}
					}
					if (dstoffs >= leng) return Result;
					header <<= 1;
				}
			}
		}
	}
}