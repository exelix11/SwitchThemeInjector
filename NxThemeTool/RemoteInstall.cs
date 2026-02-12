using System.Net.Sockets;
using System.Text;

namespace NxThemeTool
{
    internal class RemoteInstall
    {
        public static string? DoRemoteInstall(string Ip, byte[] theme)
        {
            var mem = new MemoryStream();
            BinaryWriter bin = new BinaryWriter(mem, UTF8Encoding.ASCII);
            bin.Write(Encoding.ASCII.GetBytes("theme"));
            bin.Write(new byte[3]);
            bin.Write(theme.Length);
            bin.Write(theme);
            try
            {
                Socket sock =
                    new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                var arr = mem.ToArray();

                sock.Connect(Ip, 5000);

                if (sock.Connected)
                {
                    sock.Send(arr, SocketFlags.None);

                    byte[] by = new byte[2];
                    if (sock.Receive(by, SocketFlags.None) != 2)
                        return "Didn't receive confirmation from switch :(";

                    sock.Close();

                }
                else
                    return "Socket didn't connect";
            }
            catch (Exception ex)
            {
                return "There was an error: " + ex.ToString();
            }
            return null;
        }
    }
}
