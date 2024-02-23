using System;
using System.Collections.Generic;
using UnityEngine;

namespace Story
{
    public class StoryEventManager : MonoBehaviour
    {
        [SerializeField, Tooltip("イベントトリガーのリスト")]
        List<EventTriggerInfo> _eventTriggerInfos;
        void Update()
        {
            foreach (var info in _eventTriggerInfos)
            {
                if(info.IsTriggered) continue;
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
        public bool IsTriggered { get; set; }
    }
}