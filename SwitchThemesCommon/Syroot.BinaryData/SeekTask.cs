using System;
using System.IO;

namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents a temporary seek to another position which is undone after the task has been disposed.
    /// </summary>
    public class SeekTask : IDisposable
    {
        // ---- CONSTRUCTORS -------------------------------------------------------------------------------------------

        /// <summary>
        /// Initializes a new instance of the <see cref="SeekTask"/> class to temporarily seek the given
        /// <see cref="Stream"/> to the specified position. The <see cref="System.IO.Stream"/> is rewound to its
        /// previous position after the task is disposed.
        /// </summary>
        /// <param name="stream">A <see cref="System.IO.Stream"/> to temporarily seek.</param>
        /// <param name="offset">A byte offset relative to the origin parameter.</param>
        /// <param name="origin">A value of type <see cref="SeekOrigin"/> indicating the reference point used to obtain
        /// the new position.</param>
        public SeekTask(Stream stream, long offset, SeekOrigin origin)
        {
            Stream = stream;
            PreviousPosition = stream.Position;
            Stream.Seek(offset, origin);
        }

        // ---- PROPERTIES ---------------------------------------------------------------------------------------------

        /// <summary>
        /// Gets the <see cref="Stream"/> which is temporarily sought to another position.
        /// </summary>
        public Stream Stream
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets the absolute position to which the <see cref="Stream"/> will be rewound after this task is disposed.
        /// </summary>
        public long PreviousPosition
        {
            get;
            private set;
        }

        // ---- METHODS (PUBLIC) ---------------------------------------------------------------------------------------

        /// <summary>
        /// Rewinds the <see cref="Stream"/> to its previous position.
        /// </summary>
        public void Dispose()
        {
            Stream.Seek(PreviousPosition, SeekOrigin.Begin);
        }
    }
}
