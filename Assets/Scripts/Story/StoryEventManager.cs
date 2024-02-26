using System;
using System.Collections.Generic;
using UnityEngine;

namespace Story
{
    public class StoryEventManager : MonoBehaviour
    {
        // TODO: 保存機能追加
        [SerializeField, Tooltip("イベントトリガーのリスト")]
        List<EventTriggerInfo> _eventTriggerInfos;

        public List<EventTriggerInfo> EventTriggerInfos => _eventTriggerInfos;

        private void Update()
        {
            foreach (var info in _eventTriggerInfos)
            {
                if (info.IsTriggered) continue;
                if (info.EventTrigger.CheckStoryEvent())
                {
                    info.IsTriggered = true;
                }
            }
        }
    }

    [Serializable]
    public class EventTriggerInfo
    {
        [field: SerializeField] public StoryController EventTrigger { get; set; }
        [field: SerializeField] public bool IsTriggered { get; set; }
    }
}