using System;
using System.Collections.Generic;
using UnityEngine;

namespace Story
{
    public class StoryTextManager : SingletonBase<StoryTextManager>
    {
        [SerializeField] private StoryTextController _storyTextController;
        [SerializeField] private OptionController _optionController;
        private List<string> _texts = new();
        private int _optionCount;
        private List<StoryFlagEnum> _storyFlags = new();
        private bool _isButtonPush;
        private List<IEventClip> _optionEvents = new();
        
        [Flags]
        public enum StoryFlagEnum
        {
            S11 = 1 << 1,
            S12 = 1 << 2,
            S21 = 1 << 3,
            S22 = 1 << 4,
            S23 = 1 << 5,
            S31 = 1 << 6,
            S32 = 1 << 7,
            S41 = 1 << 8,
            S42 = 1 << 9,
            S43 = 1 << 10
        }

        public StoryFlagEnum StoryFlag { get; private set; }
        
        /// <summary>
        /// ボタンに登録して、どの選択肢が選ばれたかを判定し、処理を実行するメソッド
        /// </summary>
        /// <param name="num">ボタンの番号 == 選択肢の番号</param>
        public void AddFlag(int num)
        {
            if (_isButtonPush) return;
            _isButtonPush = true;
            StoryFlag |= _storyFlags[num];
            _optionEvents[num].StartEvent();

            ResetStoryTexts();
            return;

            // ストーリー終了時処理
            void ResetStoryTexts()
            {
                _isButtonPush = false;
                _storyFlags.Clear();
                _texts.Clear();
                _optionEvents.Clear();
                _optionCount = 0;
            }
        }
        

        /// <summary>
        /// 登録された選択肢の"ストーリーフラグ","テキスト","選択時の処理"を一時的に格納するメソッド
        /// </summary>
        /// <param name="optionFlagEnum">ストーリーフラグ</param>
        /// <param name="optionText">表示テキスト</param>
        /// <param name="eventClip">選択時の処理</param>
        public void UpdateOption(StoryFlagEnum optionFlagEnum, string optionText, IEventClip eventClip)
        {
            _storyFlags.Add(optionFlagEnum);
            _texts.Add(optionText);
            _optionEvents.Add(eventClip);
            _optionCount++;
        }

        /// <summary>
        /// ストーリーを更新するメソッド
        /// テキスト更新が完了すると、選択肢更新を実行する
        /// </summary>
        /// <param name="speakerName">話者名前</param>
        /// <param name="texts">喋る内容</param>
        public async void UpdateText(string speakerName, List<string> texts)
        {
            await _storyTextController.UpdateTextAsync(speakerName, texts);
            _optionController.UpdateDialogue(_optionCount, _texts);
        }
    }
}