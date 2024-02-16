//  引用元 https://qiita.com/tsukimi_neko/items/7922b2433ed4d8616cce
using System;
using UnityEngine;

[AttributeUsage(AttributeTargets.Field, AllowMultiple = false)]
public class SubclassSelectorAttribute : PropertyAttribute
{
    bool m_includeMono;

    public SubclassSelectorAttribute(bool includeMono = false)
    {
        m_includeMono = includeMono;
    }

    public bool IsIncludeMono()
    {
        return m_includeMono;
    }
}