using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using Syroot.BinaryData.Core;

namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents an extended <see cref="BinaryReader"/> supporting special file format data types.
    /// </summary>
    [DebuggerDisplay("BinaryDataReader, Position={Position}")]
    public class BinaryDataReader : BinaryReader
    {
        // ---- FIELDS -------------------------------------------------------------------------------------------------

        private ByteOrder _byteOrder;
        
        // ---- CONSTRUCTORS -------------------------------------------------------------------------------------------

        /// <summary>
        /// Initializes a new instance of the <see cref="BinaryDataReader"/> class based on the specified stream and
        /// using UTF-8 encoding.
        /// </summary>
        /// <param name="input">The input stream.</param>
        /// <exception cref="ArgumentException">The stream does not support reading, is null, or is already closed.
        /// </exception>
        public BinaryDataReader(Stream input)
            : this(input, new UTF8Encoding(), false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BinaryDataReader"/> class based on the specified stream, UTF-8
        /// encoding and optionally leaves the stream open.
        /// </summary>
        /// <param name="input">The input stream.</param>
        /// <param name="leaveOpen"><c>true</c> to leave the stream open after the <see cref="BinaryDataReader"/> object
        /// is disposed; otherwise <c>false</c>.</param>
        /// <exception cref="ArgumentException">The stream does not support reading, is null, or is already closed.
        /// </exception>
        /// <exception cref="ArgumentNullException">encoding is null.</exception>
        public BinaryDataReader(Stream input, bool leaveOpen)
            : this(input, new UTF8Encoding(), leaveOpen)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BinaryDataReader"/> class based on the specified stream and
        /// character encoding.
        /// </summary>
        /// <param name="input">The input stream.</param>
        /// <param name="encoding">The character encoding to use.</param>
        /// <exception cref="ArgumentException">The stream does not support reading, is null, or is already closed.
        /// </exception>
        /// <exception cref="ArgumentNullException">encoding is null.</exception>
        public BinaryDataReader(Stream input, Encoding encoding)
            : this(input, encoding, false)
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="BinaryDataReader"/> class based on the specified stream and
        /// character encoding, and optionally leaves the stream open.
        /// </summary>
        /// <param name="input">The input stream.</param>
        /// <param name="encoding">The character encoding to use.</param>
        /// <param name="leaveOpen"><c>true</c> to leave the stream open after the <see cref="BinaryDataReader"/> object
        /// is disposed; otherwise <c>false</c>.</param>
        /// <exception cref="ArgumentException">The stream does not support reading, is null, or is already closed.
        /// </exception>
        /// <exception cref="ArgumentNullException">encoding is null.</exception>
        public BinaryDataReader(Stream input, Encoding encoding, bool leaveOpen)
            : base(input, encoding, leaveOpen)
        {
            Encoding = encoding;
            ByteOrder = ByteOrderHelper.SystemByteOrder;
        }

        // ---- PROPERTIES ---------------------------------------------------------------------------------------------

        /// <summary>
        /// Gets or sets the byte order used to parse binary data with.
        /// </summary>
        public ByteOrder ByteOrder
        {
            get
            {
                return _byteOrder;
            }
            set
            {
                _byteOrder = value;
                NeedsReversion = _byteOrder != ByteOrderHelper.SystemByteOrder;
            }
        }

        /// <summary>
        /// Gets the encoding used for string related operations where no other encoding has been provided. Due to the
        /// way the underlying <see cref="BinaryReader"/> is instantiated, it can only be specified at creation time.
        /// </summary>
        public Encoding Encoding
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets a value indicating whether the end of the stream has been reached and no more data can be read.
        /// </summary>
        public bool EndOfStream
        {
            get { return BaseStream.Position >= BaseStream.Length; }
        }

        /// <summary>
        /// Gets the length in bytes of the stream in bytes. This is a shortcut to the base stream Length property.
        /// </summary>
        public long Length
        {
            get { return BaseStream.Length; }
        }

        /// <summary>
        /// Gets a value indicating whether multibyte data requires to be reversed before being parsed, according to the
        /// set <see cref="ByteOrder"/>.
        /// </summary>
        public bool NeedsReversion
        {
            get;
            private set;
        }

        /// <summary>
        /// Gets or sets the position within the current stream. This is a shortcut to the base stream Position
        /// property.
        /// </summary>
        public long Position
        {
            get { return BaseStream.Position; }
            set { BaseStream.Position = value; }
        }

        // ---- METHODS (PUBLIC) ---------------------------------------------------------------------------------------

        /// <summary>
        /// Aligns the reader to the next given byte multiple.
        /// </summary>
        /// <param name="alignment">The byte multiple.</param>
        public void Align(int alignment)
        {
            Seek((-Position % alignment + alignment) % alignment);
        }

        /// <summary>
        /// Reads a <see cref="Boolean"/> value from the current stream. The <see cref="Boolean"/> is available in the
        /// specified binary format.
        /// </summary>
        /// <param name="format">The binary format, in which the <see cref="Boolean"/> will be read.</param>
        /// <returns>The <see cref="Boolean"/> read from the current stream.</returns>
        public Boolean ReadBoolean(BinaryBooleanFormat format)
        {
            switch (format)
            {
                case BinaryBooleanFormat.NonZeroByte:
                    return base.ReadBoolean();
                case BinaryBooleanFormat.NonZeroWord:
                    return ReadInt16() != 0;
                case BinaryBooleanFormat.NonZeroDword:
                    return ReadInt32() != 0;
                default:
                    throw new ArgumentOutOfRangeException(nameof(format),
                        "The specified binary boolean format is invalid.");
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="Boolean"/> values from the current stream into a
        /// <see cref="Boolean"/> array.
        /// </summary>
        /// <param name="count">The number of <see cref="Boolean"/> values to read.</param>
        /// <returns>The <see cref="Boolean"/> array read from the current stream.</returns>
        public Boolean[] ReadBooleans(int count)
        {
            return ReadMultiple(count, base.ReadBoolean);
        }

        /// <summary>
        /// Reads the specified number of <see cref="Boolean"/> values from the current stream into a
        /// <see cref="Boolean"/> array. The <see cref="Boolean"/> values are available in the specified binary format.
        /// </summary>
        /// <param name="count">The number of <see cref="Boolean"/> values to read.</param>
        /// <param name="format">The binary format, in which the <see cref="Boolean"/> values will be read.</param>
        /// <returns>The <see cref="Boolean"/> array read from the current stream.</returns>
        public Boolean[] ReadBooleans(int count, BinaryBooleanFormat format)
        {
            Boolean[] values = new Boolean[count];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = ReadBoolean(format);
            }
            return values;
        }

        /// <summary>
        /// Reads a <see cref="DateTime"/> from the current stream.
        /// </summary>
        /// <returns>The <see cref="DateTime"/> read from the current stream.</returns>
        public DateTime ReadDateTime()
        {
            return ReadDateTime(BinaryDateTimeFormat.NetTicks);
        }

        /// <summary>
        /// Reads a <see cref="DateTime"/> from the current stream. The <see cref="DateTime"/> is available in the
        /// specified binary format.
        /// </summary>
        /// <param name="format">The binary format, in which the <see cref="DateTime"/> will be read.</param>
        /// <returns>The <see cref="DateTime"/> read from the current stream.</returns>
        public DateTime ReadDateTime(BinaryDateTimeFormat format)
        {
            switch (format)
            {
                case BinaryDateTimeFormat.CTime:
                    return new DateTime(1970, 1, 1).ToLocalTime().AddSeconds(ReadUInt32());
                case BinaryDateTimeFormat.NetTicks:
                    return new DateTime(ReadInt64());
                default:
                    throw new ArgumentOutOfRangeException(nameof(format),
                        "The specified binary date time format is invalid.");
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="DateTime"/> values from the current stream into a
        /// <see cref="DateTime"/> array.
        /// </summary>
        /// <param name="count">The number of <see cref="DateTime"/> values to read.</param>
        /// <returns>The <see cref="DateTime"/> array read from the current stream.</returns>
        public DateTime[] ReadDateTimes(int count)
        {
            DateTime[] values = new DateTime[count];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = ReadDateTime();
            }
            return values;
        }

        /// <summary>
        /// Reads the specified number of <see cref="DateTime"/> values from the current stream into a
        /// <see cref="DateTime"/> array. The <see cref="DateTime"/> values are available in the specified binary
        /// format.
        /// </summary>
        /// <param name="count">The number of <see cref="DateTime"/> values to read.</param>
        /// <param name="format">The binary format, in which the <see cref="DateTime"/> values will be read.</param>
        /// <returns>The <see cref="DateTime"/> array read from the current stream.</returns>
        public DateTime[] ReadDateTimes(int count, BinaryDateTimeFormat format)
        {
            DateTime[] values = new DateTime[count];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = ReadDateTime(format);
            }
            return values;
        }
		
        /// <summary>
        /// Reads the specified number of <see cref="Decimal"/> values from the current stream into a
        /// <see cref="Decimal"/> array and advances the current position by that number of <see cref="Decimal"/> values
        /// multiplied with the size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="Decimal"/> values to read.</param>
        /// <returns>The <see cref="Decimal"/> array read from the current stream.</returns>
        public Decimal[] ReadDecimals(int count)
        {
            return ReadMultiple(count, ReadDecimal);
        }

        /// <summary>
        /// Reads an 8-byte floating point value from the current stream and advances the current position of the stream
        /// by eight bytes.
        /// </summary>
        /// <returns>The 8-byte floating point value read from the current stream.</returns>
        public override Double ReadDouble()
        {
            if (NeedsReversion)
            {
                byte[] bytes = base.ReadBytes(8);
                Array.Reverse(bytes);
                return BitConverter.ToDouble(bytes, 0);
            }
            else
            {
                return base.ReadDouble();
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="Double"/> values from the current stream into a
        /// <see cref="Double"/> array and advances the current position by that number of <see cref="Double"/> values
        /// multiplied with the size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="Double"/> values to read.</param>
        /// <returns>The <see cref="Double"/> array read from the current stream.</returns>
        public Double[] ReadDoubles(int count)
        {
            return ReadMultiple(count, ReadDouble);
        }
		
        /// <summary>
        /// Reads a 2-byte signed integer from the current stream and advances the current position of the stream by two
        /// bytes.
        /// </summary>
        /// <returns>The 2-byte signed integer read from the current stream.</returns>
        public override Int16 ReadInt16()
        {
            if (NeedsReversion)
            {
                byte[] bytes = base.ReadBytes(2);
                Array.Reverse(bytes);
                return BitConverter.ToInt16(bytes, 0);
            }
            else
            {
                return base.ReadInt16();
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="Int16"/> values from the current stream into a <see cref="Int16"/>
        /// array and advances the current position by that number of <see cref="Int16"/> values multiplied with the
        /// size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="Int16"/> values to read.</param>
        /// <returns>The <see cref="Int16"/> array read from the current stream.</returns>
        public Int16[] ReadInt16s(int count)
        {
            return ReadMultiple(count, ReadInt16);
        }

        /// <summary>
        /// Reads a 4-byte signed integer from the current stream and advances the current position of the stream by
        /// four bytes.
        /// </summary>
        /// <returns>The 4-byte signed integer read from the current stream.</returns>
        public override Int32 ReadInt32()
        {
            if (NeedsReversion)
            {
                byte[] bytes = base.ReadBytes(4);
                Array.Reverse(bytes);
                return BitConverter.ToInt32(bytes, 0);
            }
            else
            {
                return base.ReadInt32();
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="Int32"/> values from the current stream into a <see cref="Int32"/>
        /// array and advances the current position by that number of <see cref="Int32"/> values multiplied with the
        /// size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="Int32"/> values to read.</param>
        /// <returns>The <see cref="Int32"/> array read from the current stream.</returns>
        public Int32[] ReadInt32s(int count)
        {
            return ReadMultiple(count, ReadInt32);
        }

        /// <summary>
        /// Reads an 8-byte signed integer from the current stream and advances the current position of the stream by
        /// eight bytes.
        /// </summary>
        /// <returns>The 8-byte signed integer read from the current stream.</returns>
        public override Int64 ReadInt64()
        {
            if (NeedsReversion)
            {
                byte[] bytes = base.ReadBytes(8);
                Array.Reverse(bytes);
                return BitConverter.ToInt64(bytes, 0);
            }
            else
            {
                return base.ReadInt64();
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="Int64"/> values from the current stream into a <see cref="Int64"/>
        /// array and advances the current position by that number of <see cref="Int64"/> values multiplied with the
        /// size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="Int64"/> values to read.</param>
        /// <returns>The <see cref="Int64"/> array read from the current stream.</returns>
        public Int64[] ReadInt64s(int count)
        {
            return ReadMultiple(count, ReadInt64);
        }
        
        /// <summary>
        /// Reads the specified number of <see cref="SByte"/> values from the current stream into a <see cref="SByte"/>
        /// array and advances the current position by that number of <see cref="SByte"/> values multiplied with the
        /// size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="SByte"/> values to read.</param>
        /// <returns>The <see cref="SByte"/> array read from the current stream.</returns>
        public SByte[] ReadSBytes(int count)
        {
            return ReadMultiple(count, ReadSByte);
        }

        /// <summary>
        /// Reads a 4-byte floating point value from the current stream and advances the current position of the stream
        /// by four bytes.
        /// </summary>
        /// <returns>The 4-byte floating point value read from the current stream.</returns>
        public override Single ReadSingle()
        {
            if (NeedsReversion)
            {
                byte[] bytes = base.ReadBytes(4);
                Array.Reverse(bytes);
                return BitConverter.ToSingle(bytes, 0);
            }
            else
            {
                return base.ReadSingle();
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="Single"/> values from the current stream into a
        /// <see cref="Single"/> array and advances the current position by that number of <see cref="Single"/> values
        /// multiplied with the size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="Single"/> values to read.</param>
        /// <returns>The <see cref="Single"/> array read from the current stream.</returns>
        public Single[] ReadSingles(int count)
        {
            return ReadMultiple(count, ReadSingle);
        }

        /// <summary>
        /// Reads a string from the current stream. The string is available in the specified binary format.
        /// </summary>
        /// <param name="format">The binary format, in which the string will be read.</param>
        /// <returns>The string read from the current stream.</returns>
        public String ReadString(BinaryStringFormat format)
        {
            return ReadString(format, Encoding);
        }

        /// <summary>
        /// Reads a string from the current stream. The string is available in the specified binary format and encoding.
        /// </summary>
        /// <param name="format">The binary format, in which the string will be read.</param>
        /// <param name="encoding">The encoding used for converting the string.</param>
        /// <returns>The string read from the current stream.</returns>
        public String ReadString(BinaryStringFormat format, Encoding encoding)
        {
            switch (format)
            {
                case BinaryStringFormat.ByteLengthPrefix:
                    return ReadStringInternal(ReadByte(), encoding);
                case BinaryStringFormat.WordLengthPrefix:
                    return ReadStringInternal(ReadInt16(), encoding);
                case BinaryStringFormat.DwordLengthPrefix:
                    return ReadStringInternal(ReadInt32(), encoding);
                case BinaryStringFormat.VariableLengthPrefix:
                    return ReadStringInternal(Read7BitEncodedInt(), encoding);
                case BinaryStringFormat.ZeroTerminated:
                    return ReadZeroTerminatedString(encoding);
                case BinaryStringFormat.NoPrefixOrTermination:
                    throw new ArgumentException("NoPrefixOrTermination cannot be used for read operations if no length "
                        + "has been specified.", nameof(format));
                default:
                    throw new ArgumentOutOfRangeException(nameof(format),
                        "The specified binary string format is invalid.");
            }
        }

        /// <summary>
        /// Reads a string from the current stream. The string has neither a prefix or postfix, the length has to be
        /// specified manually.
        /// </summary>
        /// <param name="length">The length of the string.</param>
        /// <returns>The string read from the current stream.</returns>
        public String ReadString(int length)
        {
            return ReadString(length, Encoding);
        }

        /// <summary>
        /// Reads a string from the current stream. The string has neither a prefix or postfix, the length has to be
        /// specified manually. The string is available in the specified encoding.
        /// </summary>
        /// <param name="length">The length of the string.</param>
        /// <param name="encoding">The encoding to use for reading the string.</param>
        /// <returns>The <see cref="String"/> read from the current stream.</returns>
        public String ReadString(int length, Encoding encoding)
        {
            return encoding.GetString(ReadBytes(length));
        }

        /// <summary>
        /// Reads the specified number of <see cref="String"/> from the current stream into a <see cref="String"/>
        /// array.
        /// </summary>
        /// <param name="count">The number of <see cref="String"/> values to read.</param>
        /// <returns>The <see cref="String"/> array read from the current stream.</returns>
        public String[] ReadStrings(int count)
        {
            String[] values = new String[count];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = ReadString();
            }
            return values;
        }

        /// <summary>
        /// Reads the specified number of <see cref="String"/> from the current stream into a <see cref="String"/>
        /// array. The strings are available in the specified binary format.
        /// </summary>
        /// <param name="count">The number of <see cref="String"/> values to read.</param>
        /// <param name="format">The binary format, in which the string will be read.</param>
        /// <returns>The <see cref="String"/> array read from the current stream.</returns>
        public String[] ReadStrings(int count, BinaryStringFormat format)
        {
            String[] values = new String[count];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = ReadString(format);
            }
            return values;
        }

        /// <summary>
        /// Reads the specified number of <see cref="String"/> from the current stream into a <see cref="String"/>
        /// array. The strings are available in the specified binary format and encoding.
        /// </summary>
        /// <param name="count">The number of <see cref="String"/> values to read.</param>
        /// <param name="format">The binary format, in which the string will be read.</param>
        /// <param name="encoding">The encoding used for converting the string.</param>
        /// <returns>The <see cref="String"/> array read from the current stream.</returns>
        public String[] ReadStrings(int count, BinaryStringFormat format, Encoding encoding)
        {
            String[] values = new String[count];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = ReadString(format, encoding);
            }
            return values;
        }

        /// <summary>
        /// Reads the specified number of <see cref="String"/> from the current stream into a <see cref="String"/>
        /// array. The strings have neither a prefix or postfix, the length has to be specified manually.
        /// </summary>
        /// <param name="count">The number of <see cref="String"/> values to read.</param>
        /// <param name="length">The length of the string.</param>
        /// <returns>The <see cref="String"/> array read from the current stream.</returns>
        public String[] ReadStrings(int count, int length)
        {
            String[] values = new String[count];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = ReadString(length);
            }
            return values;
        }

        /// <summary>
        /// Reads the specified number of <see cref="String"/> from the current stream into a <see cref="String"/>
        /// array. The strings have neither a prefix or postfix, the length has to be specified manually. The strings
        /// are available in the specified encoding.
        /// </summary>
        /// <param name="count">The number of <see cref="String"/> values to read.</param>
        /// <param name="length">The length of the string.</param>
        /// <param name="encoding">The encoding to use for reading the string.</param>
        /// <returns>The <see cref="String"/> array read from the current stream.</returns>
        public String[] ReadStrings(int count, int length, Encoding encoding)
        {
            String[] values = new String[count];
            for (int i = 0; i < values.Length; i++)
            {
                values[i] = ReadString(length, Encoding);
            }
            return values;
        }

        /// <summary>
        /// Reads a 2-byte unsigned integer from the current stream using little-endian encoding and advances the
        /// position of the stream by two bytes.
        /// </summary>
        /// <returns>The 2-byte unsigned integer read from the current stream.</returns>
        public override UInt16 ReadUInt16()
        {
            if (NeedsReversion)
            {
                byte[] bytes = base.ReadBytes(2);
                Array.Reverse(bytes);
                return BitConverter.ToUInt16(bytes, 0);
            }
            else
            {
                return base.ReadUInt16();
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="UInt16"/> values from the current stream into a
        /// <see cref="UInt16"/> array and advances the current position by that number of <see cref="UInt16"/> values
        /// multiplied with the size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="UInt16"/> values to read.</param>
        /// <returns>The <see cref="UInt16"/> array read from the current stream.</returns>
        public UInt16[] ReadUInt16s(int count)
        {
            return ReadMultiple(count, ReadUInt16);
        }

        /// <summary>
        /// Reads an 8-byte unsigned integer from the current stream and advances the position of the stream by eight
        /// bytes.
        /// </summary>
        /// <returns>The 8-byte unsigned integer read from the current stream.</returns>
        public override UInt32 ReadUInt32()
        {
            if (NeedsReversion)
            {
                byte[] bytes = base.ReadBytes(4);
                Array.Reverse(bytes);
                return BitConverter.ToUInt32(bytes, 0);
            }
            else
            {
                return base.ReadUInt32();
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="UInt32"/> values from the current stream into a
        /// <see cref="UInt32"/> array and advances the current position by that number of <see cref="UInt32"/> values
        /// multiplied with the size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="UInt32"/> values to read.</param>
        /// <returns>The <see cref="UInt32"/> array read from the current stream.</returns>
        public UInt32[] ReadUInt32s(int count)
        {
            return ReadMultiple(count, ReadUInt32);
        }

        /// <summary>
        /// Reads an 8-byte unsigned integer from the current stream and advances the position of the stream by eight
        /// bytes.
        /// </summary>
        /// <returns>The 8-byte unsigned integer read from the current stream.</returns>
        public override UInt64 ReadUInt64()
        {
            if (NeedsReversion)
            {
                byte[] bytes = base.ReadBytes(8);
                Array.Reverse(bytes);
                return BitConverter.ToUInt64(bytes, 0);
            }
            else
            {
                return base.ReadUInt64();
            }
        }

        /// <summary>
        /// Reads the specified number of <see cref="UInt64"/> values from the current stream into a
        /// <see cref="UInt64"/> array and advances the current position by that number of <see cref="UInt64"/> values
        /// multiplied with the size of a single value.
        /// </summary>
        /// <param name="count">The number of <see cref="UInt64"/> values to read.</param>
        /// <returns>The <see cref="UInt64"/> array read from the current stream.</returns>
        public UInt64[] ReadUInt64s(int count)
        {
            return ReadMultiple(count, ReadUInt64);
        }

        /// <summary>
        /// Sets the position within the current stream. This is a shortcut to the base stream Seek method.
        /// </summary>
        /// <param name="offset">A byte offset relative to the current position in the stream.</param>
        /// <returns>The new position within the current stream.</returns>
        public long Seek(long offset)
        {
            return Seek(offset, SeekOrigin.Current);
        }

        /// <summary>
        /// Sets the position within the current stream. This is a shortcut to the base stream Seek method.
        /// </summary>
        /// <param name="offset">A byte offset relative to the origin parameter.</param>
        /// <param name="origin">A value of type <see cref="SeekOrigin"/> indicating the reference point used to obtain
        /// the new position.</param>
        /// <returns>The new position within the current stream.</returns>
        public long Seek(long offset, SeekOrigin origin)
        {
            return BaseStream.Seek(offset, origin);
        }

        /// <summary>
        /// Creates a <see cref="SeekTask"/> to restore the current position after it has been disposed.
        /// </summary>
        /// <returns>The <see cref="SeekTask"/> to be disposed to restore to the current position.</returns>
        public SeekTask TemporarySeek()
        {
            return TemporarySeek(0, SeekOrigin.Current);
        }

        /// <summary>
        /// Creates a <see cref="SeekTask"/> with the given parameters. As soon as the returned <see cref="SeekTask"/>
        /// is disposed, the previous stream position will be restored.
        /// </summary>
        /// <param name="offset">A byte offset relative to the current position in the stream.</param>
        /// <returns>The <see cref="SeekTask"/> to be disposed to undo the seek.</returns>
        public SeekTask TemporarySeek(long offset)
        {
            return TemporarySeek(offset, SeekOrigin.Current);
        }

        /// <summary>
        /// Creates a <see cref="SeekTask"/> with the given parameters. As soon as the returned <see cref="SeekTask"/>
        /// is disposed, the previous stream position will be restored.
        /// </summary>
        /// <param name="offset">A byte offset relative to the origin parameter.</param>
        /// <param name="origin">A value of type <see cref="SeekOrigin"/> indicating the reference point used to obtain
        /// the new position.</param>
        /// <returns>The <see cref="SeekTask"/> to be disposed to undo the seek.</returns>
        public SeekTask TemporarySeek(long offset, SeekOrigin origin)
        {
            return new SeekTask(BaseStream, offset, origin);
        }

        // ---- METHODS (PRIVATE) --------------------------------------------------------------------------------------

        private T[] ReadMultiple<T>(int count, Func<T> readFunc)
        {
            T[] values = new T[count];
            for (int i = 0; i < count; i++)
            {
                values[i] = readFunc.Invoke();
            }
            return values;
        }

        // ---- String methods ----

        private string ReadStringInternal(int length, Encoding encoding)
        {
            // This will not work for strings with differently sized characters depending on their code.
            return encoding.GetString(ReadBytes(length * encoding.GetByteCount("a")));
        }

        private string ReadZeroTerminatedString(Encoding encoding)
        {
            // This will not work for strings with differently sized characters depending on their code.
            int charSize = encoding.GetByteCount("a");

            List<byte> bytes = new List<byte>();
            if (charSize == 1)
            {
                // Read single bytes.
                byte readByte = ReadByte();
                while (readByte != 0)
                {
                    bytes.Add(readByte);
                    readByte = ReadByte();
                }
            }
            else if (charSize == 2)
            {
                // Read ushort values with 2 bytes width.
                uint readUShort = ReadUInt16();
                while (readUShort != 0)
                {
                    byte[] ushortBytes = BitConverter.GetBytes(readUShort);
                    bytes.Add(ushortBytes[0]);
                    bytes.Add(ushortBytes[1]);
                    readUShort = ReadUInt16();
                }
            }

            // Convert to string.
            return encoding.GetString(bytes.ToArray());
        }
    }
}
