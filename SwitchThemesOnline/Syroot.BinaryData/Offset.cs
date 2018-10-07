namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents a space of 4 bytes reserved in the underlying stream of a <see cref="BinaryDataWriter"/> which can
    /// be comfortably satisfied later on.
    /// </summary>
    public class Offset
    {
        // ---- CONSTRUCTORS & DESTRUCTOR ------------------------------------------------------------------------------

        /// <summary>
        /// Initializes a new instance of the <see cref="Offset"/> class reserving an offset with the specified
        /// <see cref="BinaryDataWriter"/> at the current position.
        /// </summary>
        /// <param name="writer">The <see cref="BinaryDataWriter"/> holding the stream in which the offset will be
        /// reserved.</param>
        public Offset(BinaryDataWriter writer)
        {
            Writer = writer;
            Position = (uint)Writer.Position;
            Writer.Position += 4;
        }

        // ---- PROPERTIES ---------------------------------------------------------------------------------------------

        /// <summary>
        /// Gets the <see cref="BinaryDataWriter"/> in which underlying stream the allocation is made.
        /// </summary>
        public BinaryDataWriter Writer { get; private set; }

        /// <summary>
        /// Gets the address at which the allocation is made.
        /// </summary>
        public uint Position { get; private set; }

        // ---- METHODS (PUBLIC) ---------------------------------------------------------------------------------------

        /// <summary>
        /// Satisfies the offset by writing the current position of the underlying stream at the reserved
        /// <see cref="Position"/>, then seeking back to the current position.
        /// </summary>
        public void Satisfy()
        {
            Satisfy((int)Writer.Position);
        }

        /// <summary>
        /// Satisfies the offset by writing the given value of the underlying stream at the reserved
        /// <see cref="Position"/>, then seeking back to the current position.
        /// </summary>
        public void Satisfy(int value)
        {
            // Temporarily seek back to the allocation offset and write the given value there, then seek back.
            uint oldPosition = (uint)Writer.Position;
            Writer.Position = Position;
            Writer.Write(value);
            Writer.Position = oldPosition;
        }
    }
}
