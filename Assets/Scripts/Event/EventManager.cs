using System;
using System.Collections.Generic;
using UnityEngine;

public class EventManager : MonoBehaviour
{
    [SerializeField, Tooltip("イベントトリガーのリスト")]
    List<EventTriggerDatum> _eventTriggerData;

    public List<EventTriggerDatum> EventTriggerData => _eventTriggerData;
    void Update()
    {
        foreach (var info in _eventTriggerData)
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
public class EventTriggerDatum
{
    [field: SerializeField] public EventTrigger EventTrigger { get; set; }
    [field: SerializeField] public bool IsTriggered { get; set; }
}