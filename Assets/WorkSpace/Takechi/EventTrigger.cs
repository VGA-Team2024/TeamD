using System;
using UnityEngine;

public class EventTrigger
{
    EventTest _testFlag = new();

    void Test()
    {
        _testFlag.AddFlag();
        if (_testFlag.Flag.HasFlag(TestFlag.A))
        {
            Debug.Log("HasFlag");
        }
    }
}
[Flags]
public enum TestFlag
{
    A = 1 << 0,
    B = 1 << 1,
    C = 1 << 2,
    D = 1 << 3
}

public interface IEventCondition<T> where T : Enum
{
    T Flag { get; set; }
}

public class EventTest : IEventCondition<TestFlag>
{
    public TestFlag Flag { get; set; }

    public void AddFlag()
    {
        Flag |= TestFlag.A;
    }
}
