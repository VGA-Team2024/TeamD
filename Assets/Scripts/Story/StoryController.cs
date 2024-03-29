using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Cysharp.Threading.Tasks;
using UnityEditor;
using UnityEngine;
using Debug = UnityEngine.Debug;

namespace Story
{
    [CreateAssetMenu(menuName = "ScriptableObjects/StoryData")]
    public class StoryController : ScriptableObject
    {
        [field: SerializeField, ReadOnly] public string UniqueID { get; private set; }
        [Header("ストーリー再生判定")]
        [SerializeField, Tooltip("話者の名前選択")] private SpeakerNameFlag _nameFlag;
        [SerializeField, Tooltip("会話内容")] private List<string> _storyTexts;
        [SerializeReference, SubclassSelector, Tooltip("ストーリー突入条件設定")] private List<ICondition> _storyConditions;

        [Space, Header("選択肢を登録する")]
        [SerializeField] private List<StoryOption> _storyOptions;

        public List<StoryOption> StoryOptions => _storyOptions;
        public List<string> StoryTexts => _storyTexts;
        public string SpeakerName => _nameFlag.ToString();
        
        private enum SpeakerNameFlag
        {
            Grandma,
            You
        }

        [ContextMenu("Exe"), Conditional("UNITY_EDITOR")]
        private void DebugExecute()
        {
            StoryTextManager.Instance.RegisterStoryController(this);
            foreach (var option in _storyOptions)
            {
                if(option == null) continue;
                option.CheckOptionEvent();
            }
        }
        
        public bool CheckStoryEvent()
        {
            foreach (var condition in _storyConditions)
            {
                if(condition == null) continue;
                if (!condition.CheckCondition()) return false;
            }
            StoryTextManager.Instance.RegisterStoryController(this);
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
        public IEventClip EventClip => _eventClip;
        public string OptionText => _optionText;
        public StoryTextManager.StoryFlagEnum StoryFlagEnum => _storyFlagEnum;
        
        public bool CheckOptionEvent()
        {
            foreach (var condition in _optionConditions)
            {
                if(condition == null) continue;
                if (!condition.CheckCondition()) return false;
            }
            return true;
        }
    }
    
    public class AddHeavenlyChips : IEventClip
    {
        [Header("HC加算")]
        [SerializeField] private int _value;
        public void StartEvent()
        {
            StatsManager.HeavenlyChips += _value;
        }
    }
    
    public class SubtractCookie : IEventClip
    {
        [Header("クッキー減算")]
        [SerializeField] private double _value;
        
        public void StartEvent()
        {
            PlayerManager.Instance.SubtractCookie(_value);
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