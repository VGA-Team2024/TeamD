using System;
using System.Collections.Generic;
using UnityEngine;

public class EventManager : MonoBehaviour
{
    [SerializeField, Tooltip("イベントトリガーのリスト")]
    List<EventTriggerInfo> _eventTriggerInfos;
    void Update()
    {
        foreach (var info in _eventTriggerInfos)
        {
            if(info.IsTriggered) continue;
            if (info.EventTrigger.CheckEvent())
            {
                info.IsTriggered = true;
            }
        }
    }
}
[Serializable]
public class EventTriggerInfo
{
    [field: SerializeField] public EventTrigger EventTrigger { get; set; }
    public bool IsTriggered { get; set; }
}