using System;
using System.Collections.Generic;
using System.Linq;
using TeamD.Enum;
using UnityEngine;
using VContainer;
using Object = UnityEngine.Object;

[CreateAssetMenu(menuName = "ScriptableObjects/EventTrigger")][Serializable]
public class EventTrigger : ScriptableObject
{
    [SerializeField, SerializeReference, SubclassSelector] List<ICondition> _conditions;
    [SerializeReference, SubclassSelector] IEventClip _eventClip;
    public IEventClip EventClip => _eventClip;

    public bool CheckEvent()
    {
        foreach (var condition in _conditions)
        {
            if(condition == null) continue;
            if (!condition.CheckCondition()) return false;
        }
        _eventClip?.StartEvent();
        return true;
    }
}

public interface IEventClip
{
    void StartEvent();
}

public class NothingEvent : IEventClip { public void StartEvent() { } }
/// <summary>
/// 実績解放クラス
/// </summary>
public class UnlockAchievement : IEventClip
{
    [SerializeField] Achievement _unlockAchievement;
    [Inject] CanvasManager CanvasManager { get; set; }
    public void StartEvent()
    {
        StatsManager.Achievements |= _unlockAchievement;
        var popup = Object.Instantiate(CanvasManager.AchievementPopup, CanvasManager.AchievementPopupParent);
        var achievements = Resources.Load<Achievements>("Excel/Achievements");
        popup.Title.text = achievements.Entities.FirstOrDefault(e=>e.Key.HasFlag(_unlockAchievement))?.Name;
    }
}