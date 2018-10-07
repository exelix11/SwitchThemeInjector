using System;
using System.Collections;

namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents a member configuration for reading and writing it as binary data.
    /// </summary>
    [AttributeUsage(AttributeTargets.Field | AttributeTargets.Property)]
    public class BinaryMemberAttribute : Attribute
    {
        // ---- FIELDS -------------------------------------------------------------------------------------------------

        internal static readonly BinaryMemberAttribute Default = new BinaryMemberAttribute();

        // ---- PROPERTIES ---------------------------------------------------------------------------------------------
        
        /// <summary>
        /// Gets or sets offset of this field in bytes. Can be negative. Defaults to 0.
        /// </summary>
        public int Offset { get; set; }

        /// <summary>
        /// Gets or sets the origin of the offset of this field. Defaults to <see cref="OffsetOrigin.Current"/>.
        /// </summary>
        public OffsetOrigin OffsetOrigin { get; set; }

        /// <summary>
        /// Gets or sets the format for <see cref="Boolean"/> members. Defaults to
        /// <see cref="BinaryBooleanFormat.NonZeroByte"/>.
        /// </summary>
        public BinaryBooleanFormat BooleanFormat { get; set; }

        /// <summary>
        /// Gets or sets the format for <see cref="DateTime"/> members. Defaults to
        /// <see cref="BinaryDateTimeFormat.NetTicks"/>.
        /// </summary>
        public BinaryDateTimeFormat DateTimeFormat { get; set; }

        /// <summary>
        /// Gets or sets the format for <see cref="String"/> members. Defaults to
        /// <see cref="BinaryStringFormat.VariableLengthPrefix"/>.
        /// </summary>
        public BinaryStringFormat StringFormat { get; set; }
        
        /// <summary>
        /// Gets or sets the number of elements to read or write. Required for <see cref="IEnumerable"/> members or
        /// strings when <see cref="StringFormat"/> is <see cref="BinaryStringFormat.NoPrefixOrTermination"/>.
        /// Multidimensional arrays are not supported.
        /// </summary>
        public int Length { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether enum values are checked for a valid value or set of flags. Defaults
        /// to <c>false</c>.
        /// </summary>
        public bool Strict { get; set; }

        /// <summary>
        /// Gets or sets the <see cref="IBinaryConverter"/> type to read and write the value with.
        /// </summary>
        public Type Converter { get; set; }
    }

    /// <summary>
    /// Represents the origins of offsets of a member.
    /// </summary>
    public enum OffsetOrigin
    {
        /// <summary>
        /// The origin is relative to the most recent position of the data stream.
        /// </summary>
        Current,

        /// <summary>
        /// The origin is relative to the start of the class or structure, which is the position at which reading or
        /// writing the instance of the top-most base type has been initiated.
        /// </summary>
        Begin
    }
}
