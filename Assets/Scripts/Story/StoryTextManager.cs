using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
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
        [SerializeField] StoryEventManager _storyEventManager;

        private CancellationTokenSource _cts;
        private CancellationToken _ct;
        private bool _isOptionButtonPush;
        private bool _isOptionClearFlag;
        private string _speakerName;
        private Stack<StoryController> _storyBuffer = new();

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
            _cts = new CancellationTokenSource();
            _ct = _cts.Token;
        }

        /// <summary>
        /// ボタンに登録して、どの選択肢が選ばれたかを判定し、処理を実行するメソッド
        /// </summary>
        /// <param name="num">ボタンの番号 == 選択肢の番号</param>
        public void StoryEnd()
        {
            if (_isOptionButtonPush) return;
            _isOptionButtonPush = true;
            // ストーリー終了時処理
            _optionController.ResetOption();
            _isOptionButtonPush = false;
            _isStartButtonPush = false;
            if(_storyBuffer.Count <= 0) _startButton.interactable = false;
            _storyCanvas.SetActive(false);
        }
        /// <summary>
        /// ストーリー登録をして再生ボタンをインタラクト可能にする。
        /// </summary>
        public void RegisterStoryController(StoryController sc)
        {
            _storyBuffer.Push(sc);
            _startButton.interactable = true;
            _isOptionClearFlag = true;
        }

        /// <summary>
        /// ストーリー開始ボタン
        /// </summary>
        public async void StartStory()
        {
            if (_storyBuffer.TryPop(out StoryController story))
            {
                _storyCanvas.SetActive(true);
                _isStartButtonPush = true;
                try
                {
                    await _storyTextController.UpdateTextAsync(story.SpeakerName, story.StoryTexts, _cts);
                    _optionController.RegisterOption(story.StoryOptions
                        .Where(option=>option.CheckOptionEvent()).ToList(), ()=>
                    {
                        var index = _storyEventManager.EventTriggerInfos
                            .FindIndex(info => info.EventTrigger.Equals(story));
                        _storyEventManager.EventTriggerInfos[index].IsStoryEnded = true;
                        StoryEnd();
                    });
                }
                catch (OperationCanceledException)
                {
                }
            }
        }
    }
}