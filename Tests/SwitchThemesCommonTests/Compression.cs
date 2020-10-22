using Microsoft.VisualStudio.TestTools.UnitTesting;
using NUnit.Framework.Internal;
using SwitchThemes.Common;
using SwitchThemes.Common.Bflyt;
using System;
using System.Collections;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

namespace SwitchThemesCommonTests
{
	[TestClass]
	public class Compression
	{
		HashUtil hash = new HashUtil();

		private static byte[] MakeData() 
		{
			MemoryStream mem = new MemoryStream();
			BinaryWriter bin = new BinaryWriter(mem);

			foreach (char c in "Hello word, here's some data")
				bin.Write(c);
			
			foreach (int a in Enumerable.Range(0, 100))
				bin.Write(a);

			return mem.ToArray();
		}

		[TestMethod]
		// Not an important test, just to detect changes in the behavior
		public void ConsistentCompression()
		{
			Assert.AreEqual(hash.StringHash(ManagedYaz0.Compress(MakeData(), 9)), "7865BE4B54FBFE3ED21DEA9CB1E184F0F305404251203AD9EFDFA264280CD0FD");
		}

		[TestMethod]
		public void CompressionDecompression()
		{
			var data = MakeData();
			var dec = ManagedYaz0.Decompress(ManagedYaz0.Compress(data, 9));

			if (!data.SequenceEqual(dec))
				throw new Exception();
		}
	}
}
