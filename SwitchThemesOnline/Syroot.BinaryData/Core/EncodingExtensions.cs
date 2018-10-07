using System.Text;

namespace Syroot.BinaryData.Core
{
    /// <summary>
    /// Represents a set of extension methods for the <see cref="Encoding"/> class.
    /// </summary>
    internal static class EncodingExtensions
    {
        /// <summary>
        /// When overridden in a derived class, decodes all the bytes in the specified byte array into a string.
        /// </summary>
        /// <param name="encoding">The extended <see cref="Encoding"/> instance.</param>
        /// <param name="bytes">The byte array containing the sequence of bytes to decode.</param>
        /// <returns>A string that contains the results of decoding the specified sequence of bytes.</returns>
        /// <remarks>Required as this shortcut method is not included in .NET Standard 1.1.</remarks>
        internal static string GetString(this Encoding encoding, byte[] bytes)
        {
            return encoding.GetString(bytes, 0, bytes.Length);
        }
    }
}