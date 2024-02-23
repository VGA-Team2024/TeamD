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
        [SerializeField, Tooltip("話者の名前選択")] private SpeakerNameFlag _nameFlag;
        [SerializeField, Tooltip("会話内容")] private List<string> _storyTexts;
        [SerializeReference, SubclassSelector, Tooltip("ストーリー突入条件設定")] private List<ICondition> _storyConditions;

        [Space, Header("選択肢を登録する")]
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
        [SerializeField, Tooltip("選択肢のフラグ登録")] private StoryTextManager.StoryFlagEnum _storyFlagEnum;
        [SerializeField, Tooltip("選択肢のテキスト登録")] private string _optionText;
        [SerializeReference, SubclassSelector, Tooltip("選択肢の表示条件")] private List<ICondition> _optionConditions;
        [SerializeReference, SubclassSelector, Tooltip("選択肢を選択した際の処理登録")] private IEventClip _eventClip;
        
        public bool CheckOptionEvent()
        {
            foreach (var condition in _optionConditions)
            {
                if(condition == null) continue;
                if (!condition.CheckCondition()) return false;
            }
            // TODO 直でStoryTextManagerを参照するのをやめたい
            StoryTextManager.Instance.UpdateOption(_storyFlagEnum, _optionText, _eventClip);
            
            return true;
        }
    }

    // TODO 実際にHC増やす処理かく
    public class AddHeavenlyChips : IEventClip
    {
        [Header("HC加算")]
        [SerializeField] private int _value;
        public void StartEvent()
        {
            Debug.Log($"HC増える {_value}");
        }
    }

    // TODO 実際にクッキー減らす処理かく
    public class SubtractCookie : IEventClip
    {
        [Header("クッキー減算")]
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