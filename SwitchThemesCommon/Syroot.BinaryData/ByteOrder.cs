using System;

namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents the possible endianness of binary data.
    /// </summary>
    public enum ByteOrder : ushort
    {
        /// <summary>
        /// The binary data is present in big endian.
        /// </summary>
        BigEndian = 0xFEFF,

        /// <summary>
        /// The binary data is present in little endian.
        /// </summary>
        LittleEndian = 0xFFFE
    }

    /// <summary>
    /// Represents helper methods to handle data byte order.
    /// </summary>
    public static class ByteOrderHelper
    {
        // ---- FIELDS -------------------------------------------------------------------------------------------------

        private static ByteOrder _systemByteOrder;

        // ---- PROPERTIES ---------------------------------------------------------------------------------------------

        /// <summary>
        /// Gets the <see cref="ByteOrder"/> of the system executing the assembly.
        /// </summary>
        public static ByteOrder SystemByteOrder
        {
            get
            {
                if (_systemByteOrder == 0)
                {
                    _systemByteOrder = BitConverter.IsLittleEndian ? ByteOrder.LittleEndian : ByteOrder.BigEndian;
                }
                return _systemByteOrder;
            }
        }
    }
}
