using System;
using System.Collections.Generic;
using System.Linq;
using TeamD.Enum;
using UnityEditor;
using UnityEngine;
using VContainer;
using Object = UnityEngine.Object;

[CreateAssetMenu(menuName = "ScriptableObjects/EventTrigger")][Serializable]
public class EventTrigger : ScriptableObject
{
    [field: SerializeField, ReadOnly] public string UniqueID { get; private set; }
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
    void OnValidate()
    {
#if UNITY_EDITOR
        if (UniqueID == "")
        {
            UniqueID = GUID.Generate().ToString();
            EditorUtility.SetDirty(this);
        }
#endif
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
    [SerializeField, Tooltip("解放する実績、1つだけ選ばないとバグる")] Achievement _unlockAchievement;
    CanvasManager _canvasManager;
    [Inject]
    public void Construct(CanvasManager canvasManager)
    {
        _canvasManager = canvasManager;
    }
    public void StartEvent()
    {
        if (StatsManager.Achievements.HasFlag(_unlockAchievement)) return;
        StatsManager.Achievements |= _unlockAchievement;
        var popup = Object.Instantiate(_canvasManager.AchievementPopup, _canvasManager.AchievementPopupParent);
        var achievements = Resources.Load<Achievements>("Excel/Achievements");
        popup.Title.text = achievements.Entities.FirstOrDefault(e=>e.Key.HasFlag(_unlockAchievement))?.Name;
    }
}