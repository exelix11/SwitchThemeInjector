using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Text;

namespace SwitchThemesCommonTests
{
	static class Util 
	{
		public static byte[] ReadData(string relativePath) =>
			File.ReadAllBytes($"../../../../Cases/{relativePath}");

		public static string ReadString(string relativePath) =>
			File.ReadAllText($"../../../../Cases/{relativePath}");

		public static bool Exists(string relativePath) =>
			File.Exists($"../../../../Cases/{relativePath}");
	}

	class HashUtil : IDisposable
	{
		SHA256 sha = SHA256.Create();

		public void Dispose()
		{
			sha.Dispose();
		}

		public byte[] Hash(byte[] data) =>
			sha.ComputeHash(data);

		public string StringHash(byte[] data)
		{
			var hash = sha.ComputeHash(data);
			StringBuilder sb = new StringBuilder();

			foreach (var b in hash)
				sb.Append(b.ToString("X2"));

			return sb.ToString();
		}
	}
}
