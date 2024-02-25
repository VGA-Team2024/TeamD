using System;
using System.Collections.Generic;
using System.Threading;
using Cysharp.Threading.Tasks;
using UnityEngine;
using UnityEngine.UI;

namespace Story
{
    public class StoryTextManager : SingletonBase<StoryTextManager>
    {
        [SerializeField] private Button _startButton;
        [SerializeField] private bool _isStartButtonPush;
        [SerializeField] private StoryTextController _storyTextController;
        [SerializeField] private OptionController _optionController;
        [SerializeField] private GameObject _storyCanvas;

        private List<string> _texts = new();
        private int _optionCount;
        private List<StoryFlagEnum> _storyFlags = new();
        private List<IEventClip> _optionEvents = new();
        private CancellationToken _ct;
        private bool _isOptionButtonPush;

        // TODO: 保存機能追加
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

        public StoryFlagEnum StoryFlag { get; set; }
        public bool IsStartButtonPush => _isStartButtonPush;

        private void Start()
        {
            _storyCanvas.SetActive(false);
            _ct = this.GetCancellationTokenOnDestroy();
        }
        
        /// <summary>
        /// ボタンに登録して、どの選択肢が選ばれたかを判定し、処理を実行するメソッド
        /// </summary>
        /// <param name="num">ボタンの番号 == 選択肢の番号</param>
        public void AddFlag(int num)
        {
            if (_isOptionButtonPush) return;
            _isOptionButtonPush = true;
            StoryFlag |= _storyFlags[num];
            _optionEvents[num].StartEvent();

            ResetStoryTexts();
            return;

            // ストーリー終了時処理
            void ResetStoryTexts()
            {
                _isOptionButtonPush = false;
                _isStartButtonPush = false;
                _storyFlags.Clear();
                _texts.Clear();
                _optionEvents.Clear();
                _startButton.interactable = false;
                _optionCount = 0;
                _storyCanvas.SetActive(false);
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
            _startButton.interactable = true;
            try
            {
                await UniTask.WaitUntil(() => _isStartButtonPush, cancellationToken: _ct);
                await _storyTextController.UpdateTextAsync(speakerName, texts);
                _optionController.UpdateDialogue(_optionCount, _texts);
            }
            catch (OperationCanceledException)
            {
            }
        }

        /// <summary>
        /// ストーリー開始ボタン
        /// </summary>
        public void StartStory()
        {
            _storyCanvas.SetActive(true);
            _isStartButtonPush = true;
        }
    }
}