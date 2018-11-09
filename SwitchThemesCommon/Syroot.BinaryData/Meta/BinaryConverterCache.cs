using System;
using System.Collections.Generic;

namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents a cache for <see cref="IBinaryConverter"/> instances.
    /// </summary>
    internal static class BinaryConverterCache
    {
        // ---- FIELDS -------------------------------------------------------------------------------------------------

        private static readonly Dictionary<Type, IBinaryConverter> _cache = new Dictionary<Type, IBinaryConverter>();

        // ---- METHODS (INTERNAL) -------------------------------------------------------------------------------------

        /// <summary>
        /// Gets a possibly cached instance of a <see cref="IBinaryConverter"/> of the given <paramref name="type"/>.
        /// </summary>
        /// <param name="type">The <see cref="Type"/> of the <see cref="IBinaryConverter"/> to return.</param>
        /// <returns>An instance of the <see cref="IBinaryConverter"/>.</returns>
        internal static IBinaryConverter GetConverter(Type type)
        {
            if (!_cache.TryGetValue(type, out IBinaryConverter converter))
            {
                converter = (IBinaryConverter)Activator.CreateInstance(type);
                _cache.Add(type, converter);
            }
            return converter;
        }
    }
}
