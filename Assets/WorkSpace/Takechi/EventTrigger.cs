using System;
using System.Collections.Generic;
using TeamD.Enum;
using UniRx;
using UnityEngine;

[CreateAssetMenu(menuName = "ScriptableObjects/EventTrigger")]
public class EventTrigger : ScriptableObject
{
    [SerializeField] List<Condition> _conditions;
    [SerializeReference, SubclassSelector] IEventClip _eventClip;
    public void CheckEvent(ref ReactiveDictionary<string, int> intParams, Achievement achievement)
    {
        foreach (var condition in _conditions)
        {
            if (!condition.CheckCondition(ref intParams, achievement)) return;
        }
        _eventClip?.StartEvent();
    }
}

public interface IEventClip
{
    void StartEvent();
}

public class Nothing : IEventClip
{
    public void StartEvent()
    {
        Debug.Log("Event Nothing");
    }
}
public class UnlockAchievement : IEventClip
{
    [SerializeField] Achievement _unlockAchievement;
    public void StartEvent()
    {
        StatsManager.AchievementStats |= _unlockAchievement;
    }
}