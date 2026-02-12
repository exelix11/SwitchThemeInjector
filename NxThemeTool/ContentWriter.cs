using System;
using System.IO.Compression;
using System.Text;

namespace NxThemeTool
{
    public interface IContentWriter : IDisposable
    {
        void WriteFile(string name, byte[] data);
        void WriteString(string name, string data) => WriteFile(name, Encoding.UTF8.GetBytes(data));
    }

    public class DirectoryContentWriter : IContentWriter
    {
        private readonly string directoryPath;

        public DirectoryContentWriter(string directoryPath)
        {
            this.directoryPath = directoryPath;
            Directory.CreateDirectory(directoryPath);
        }

        public void WriteFile(string name, byte[] data)
        {
            var filePath = Path.Combine(directoryPath, name);
            var fileDir = Path.GetDirectoryName(filePath);
            if (fileDir != null)
                Directory.CreateDirectory(fileDir);
            File.WriteAllBytes(filePath, data);
        }

        public void Dispose()
        {
            // No resources to dispose in this implementation
        }
    }

    public class ZipContentWriter : IContentWriter
    {
        private readonly ZipArchive zip;

        public ZipContentWriter(Stream outputStream)
        {
            zip = new ZipArchive(outputStream, ZipArchiveMode.Create, leaveOpen: true);
        }

        public void WriteFile(string name, byte[] data)
        {
            var entry = zip.CreateEntry(name);
            using var entryStream = entry.Open();
            entryStream.Write(data, 0, data.Length);
        }

        public void Dispose()
        {
            zip.Dispose();
        }
    }
}
