using System.IO.Compression;

namespace NxThemeTool
{
    internal class Util
    {
        const string Image = "H4sIAAFEjmkA/+3RN1BUQRwG8P++3X27792BPJKhcTgORTuCsXEAPYUOxFgJptCBAXXGAsx0GEMHptChYqpEMVZiBGzEGCowhtF57DqojYVaOt93+5t5M7t78/9mw67wEaWUJUoTxBijBeZH4UMqIYcxuwbDJbdxheBSuUpZ2ot4WvtaKT/J9yNRE+UlD0mKJttv+yf2ur1lVlQrbU/8XcKLFGjnliTOYuQEjAcs7KCRREyaae3Ag2EOF9I1I/kRc6AtxYzPuWOGlUKY3Q1mn0QgU7Pyity08ioVq0nPr29q1tnFre0ZFZ198YLq2gbPzxw6bPiInFGjc8eMLRw3fsLESZNLpk5LTJ9RWjazctbsOXPnzV+4aPGSpcuWr1i5avWaurXr1m/ctHnL1m3bG3fs3LV7z959+w+0HDx0+MjRY8dPnDx1uu3M2XPnL1y63HHl6rXrN27evnP33v0HXd09vY+fPH32/MXLV6/737x99/7Dx0+fv9hezPT8kd/2Ckwvx7yBULYXc+rsgUDIrDw3tahcVdWkxfLrdXpxU3Nru5ddUNGXUV3b6WfGC3tz+m21783+rFjDPzX7WexXrx6KcmYejwc0hb59bWnMJQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAPiv8LB7APHc0XmzOgAA";

        public static byte[] CreateEmpty720PJPG() 
        {
            using var result = new MemoryStream();
            
            using var compressed = new MemoryStream(Convert.FromBase64String(Image));
            using (var gzip = new GZipStream(compressed, CompressionMode.Decompress))
                gzip.CopyTo(result);

            return result.ToArray();
        }
    }
}
