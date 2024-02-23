using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace Story
{
    [CreateAssetMenu(menuName = "ScriptableObjects/StoryData")]
    public class StoryController : ScriptableObject
    {
        [Header("ストーリー再生判定")]
        [SerializeField] private SpeakerNameFlag _nameFlag;
        [SerializeField] private List<string> _storyTexts;
        [SerializeField, SerializeReference, SubclassSelector] private List<ICondition> _storyConditions;

        [Space, Header("選択肢再生判定")]
        [SerializeField] private List<StoryOption> _storyOptions;
        
        private enum SpeakerNameFlag
        {
            BBA,
            You
        }
        
        public bool CheckStoryEvent()
        {
            foreach (var condition in _storyConditions)
            {
                if(condition == null) continue;
                if (!condition.CheckCondition()) return false;
            }
            StoryTextManager.Instance.UpdateText(_nameFlag.ToString(), _storyTexts);
            foreach (var condition in _storyOptions)
            {
                if(condition == null) continue;
                if (!condition.CheckOptionEvent()) return false;
            }

            return true;
        }
    }

    public interface IOptionEvent
    {
        public bool CheckOptionEvent();
    }
    
    [Serializable]
    public class StoryOption : IOptionEvent
    {
        [SerializeField] private StoryTextManager.StoryFlagEnum _storyFlagEnum;
        [SerializeField] private string _optionText;
        [SerializeReference, SubclassSelector] private List<ICondition> _optionConditions;
        [SerializeReference, SubclassSelector] private IEventClip _eventClip;
        
        public bool CheckOptionEvent()
        {
            foreach (var condition in _optionConditions)
            {
                if(condition == null) continue;
                if (!condition.CheckCondition()) return false;
            }
            StoryTextManager.Instance.UpdateOption(_storyFlagEnum, _optionText, _eventClip);
            
            return true;
        }
    }

    public class AddHeavenlyChips : IEventClip
    {
        [SerializeField] private int _value;
        public void StartEvent()
        {
            Debug.Log($"HC増える {_value}");
        }
    }

    public class SubtractCookie : IEventClip
    {
        [SerializeField] private double _value;
        public void StartEvent()
        {
            Debug.Log($"Cookie減る {_value}");
        }
    }
    
    public class StoryConditionOr : ICondition
    {
        [Header("視聴済みストーリーで条件判定(論理和)")]
        [SerializeField, Tooltip("要求ストーリーフラグ")] List<StoryTextManager.StoryFlagEnum> _requiredStoryFlagEnums;
        public bool CheckCondition()
        {
            return _requiredStoryFlagEnums.Any(flag => (StoryTextManager.Instance.StoryFlag & flag) == flag);
        }
    }
    
    public class StoryConditionAnd : ICondition
    {
        [Header("視聴済みストーリーで条件判定(論理積)")]
        [SerializeField, Tooltip("要求ストーリーフラグ")] List<StoryTextManager.StoryFlagEnum> _requiredStoryFlagEnums;
        public bool CheckCondition()
        {
            return _requiredStoryFlagEnums.All(flag => (StoryTextManager.Instance.StoryFlag & flag) == flag);
        }
    }
} 