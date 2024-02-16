using UnityEngine;

public enum CompareOptions
{
    /// <summary>以上 &gt;=</summary>
    [InspectorName(">=")] GreaterEqual,
    /// <summary>より大きい &gt;</summary>
    [InspectorName(">")] GreaterThan,
    /// <summary>以下 &lt;=</summary>
    [InspectorName("<=")] LessEqual,
    /// <summary>未満 &lt;</summary>
    [InspectorName("<")] LessThan,
    /// <summary>等しい</summary>
    [InspectorName("=")] Equals,
    /// <summary>等しくない</summary>
    [InspectorName("!=")] NotEquals
}