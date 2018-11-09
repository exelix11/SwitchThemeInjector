namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents a converter for reading and writing custom binary values.
    /// </summary>
    public interface IBinaryConverter
    {
        // ---- METHODS ------------------------------------------------------------------------------------------------

        /// <summary>
        /// Reads the value from the given <paramref name="reader"/> and returns it to set the corresponding member.
        /// </summary>
        /// <param name="reader">The <see cref="BinaryDataReader"/> to read the value from.</param>
        /// <param name="instance">The instance to which the value belongs.</param>
        /// <param name="memberAttribute">The <see cref="BinaryMemberAttribute"/> containing configuration which can be
        /// used to modify the behavior of the converter.</param>
        /// <returns>The read value.</returns>
        object Read(BinaryDataReader reader, object instance, BinaryMemberAttribute memberAttribute);

        /// <summary>
        /// Writes the value with the given <paramref name="writer"/>.
        /// </summary>
        /// <param name="writer">The <see cref="BinaryDataWriter"/> to write the value with.</param>
        /// <param name="instance">The instance to which the value belongs.</param>
        /// <param name="memberAttribute">The <see cref="BinaryMemberAttribute"/> containing configuration which can be
        /// used to modify the behavior of the converter.</param>
        /// <param name="value">The value to write.</param>
        void Write(BinaryDataWriter writer, object instance, BinaryMemberAttribute memberAttribute, object value);
    }
}
