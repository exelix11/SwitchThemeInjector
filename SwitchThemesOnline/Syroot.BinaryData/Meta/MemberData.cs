using System;
using System.Diagnostics;
using System.Reflection;

namespace Syroot.BinaryData
{
    /// <summary>
    /// Represents information on a member of a type cached as <see cref="TypeInfo"/>.
    /// </summary>
    [DebuggerDisplay(nameof(MemberData) + " " + nameof(MemberInfo) + "={" + nameof(MemberInfo) + "}")]
    internal class MemberData
    {
        // ---- CONSTRUCTORS & DESTRUCTOR ------------------------------------------------------------------------------

        /// <summary>
        /// Initializes a new instance of the <see cref="MemberData"/> class for the given <paramref name="memberInfo"/>
        /// with the specified <paramref name="attribute"/> configuration.
        /// </summary>
        /// <param name="memberInfo">The <see cref="MemberData"/> to represent.</param>
        /// <param name="type">The type of the value stored by the member.</param>
        /// <param name="attribute">The <see cref="BinaryMemberAttribute"/> configuration.</param>
        internal MemberData(MemberInfo memberInfo, Type type, BinaryMemberAttribute attribute)
        {
            MemberInfo = memberInfo;
            Type = type;
            Attribute = attribute;
        }

        // ---- PROPERTIES ---------------------------------------------------------------------------------------------

        /// <summary>
        /// Gets the <see cref="MemberInfo"/> represented.
        /// </summary>
        internal MemberInfo MemberInfo { get; }

        /// <summary>
        /// Gets the <see cref="Type"/> of the value stored by the member.
        /// </summary>
        internal Type Type { get; }

        /// <summary>
        /// Gets the <see cref="BinaryMemberAttribute"/> configuration.
        /// </summary>
        internal BinaryMemberAttribute Attribute { get; }
    }
}
