namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents the set of formats of binary boolean encodings.
    /// </summary>
    public enum BinaryBooleanFormat
    {
        /// <summary>
        /// The boolean is stored in 1 byte and is <c>true</c> when the value is not 0. This is the .NET default.
        /// </summary>
        NonZeroByte,

        /// <summary>
        /// The boolean is stored in 2 bytes and is <c>true</c> when the value is not 0.
        /// </summary>
        NonZeroWord,

        /// <summary>
        /// The boolean is stored in 4 bytes and is <c>true</c> when the value is not 0.
        /// </summary>
        NonZeroDword
    }

    /// <summary>
    /// Represents the set of formats of binary date and time encodings.
    /// </summary>
    public enum BinaryDateTimeFormat
    {
        /// <summary>
        /// The <see cref="System.DateTime"/> is stored as the ticks of a .NET <see cref="System.DateTime"/> instance.
        /// </summary>
        NetTicks,

        /// <summary>
        /// The <see cref="System.DateTime"/> has the time_t format of the C library.
        /// </summary>
        CTime
    }

    /// <summary>
    /// Represents the set of formats of binary string encodings.
    /// </summary>
    public enum BinaryStringFormat
    {
        /// <summary>
        /// The string has a prefix of variable size determining the length of the string and no postfix. This is the
        /// .NET default.
        /// </summary>
        VariableLengthPrefix,

        /// <summary>
        /// The string has a prefix of 1 byte determining the length of the string and no postfix.
        /// </summary>
        ByteLengthPrefix,

        /// <summary>
        /// The string has a prefix of 2 bytes determining the length of the string and no postfix.
        /// </summary>
        WordLengthPrefix,

        /// <summary>
        /// The string has a prefix of 4 bytes determining the length of the string and no postfix.
        /// </summary>
        DwordLengthPrefix,

        /// <summary>
        /// The string has no prefix and is terminated with a byte of the value 0.
        /// </summary>
        ZeroTerminated,

        /// <summary>
        /// The string has neither prefix nor postfix. This format is only valid for writing strings. For reading
        /// strings, the length has to be specified manually.
        /// </summary>
        NoPrefixOrTermination
    }
}
