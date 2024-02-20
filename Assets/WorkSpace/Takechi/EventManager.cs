using System;
using System.Collections;
using System.Collections.Generic;
using TeamD.Enum;
using UniRx;
using UnityEngine;

public class EventManager : MonoBehaviour
{
    [SerializeField, Tooltip("イベントトリガーのリスト")]
    List<EventTrigger> _eventTriggers;
    
    ReactiveDictionary<string, int> _intParameters = new();
    ReactiveProperty<Achievement> _achievementParameter = new();

    void Awake()
    {
        foreach (var eventTrigger in _eventTriggers)
        {
            _intParameters.ObserveReplace()
                .Subscribe(_=>eventTrigger.CheckEvent(ref _intParameters, _achievementParameter.Value)).AddTo(this);
            _intParameters.ObserveCountChanged()
                .Subscribe(_=>eventTrigger.CheckEvent(ref _intParameters, _achievementParameter.Value)).AddTo(this);
            _achievementParameter
                .Subscribe(_=>eventTrigger.CheckEvent(ref _intParameters, _achievementParameter.Value)).AddTo(this);
        }
    }

    public void SetInteger(string key, int value)
    {
        _intParameters[key] = value;
    }

    public void SetAchievement(Achievement achievement)
    {
        _achievementParameter.Value = achievement;
    }
}
