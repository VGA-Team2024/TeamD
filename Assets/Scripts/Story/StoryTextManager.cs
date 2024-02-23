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
        
        public void AddFlag(int num)
        {
            if (_isButtonPush) return;
            _isButtonPush = true;
            StoryFlag |= _storyFlags[num];
            _optionEvents[num].StartEvent();
        }

        public void UpdateOption(StoryFlagEnum optionFlagEnum, string optionText, IEventClip eventClip)
        {
            _storyFlags.Add(optionFlagEnum);
            _texts.Add(optionText);
            _optionEvents.Add(eventClip);
            _optionCount++;
        }

        public async void UpdateText(string speakerName, List<string> texts)
        {
            await _storyTextController.TestUpdateText(speakerName, texts);
            _optionController.UpdateDialogue(_optionCount, _texts);
        }
    }
}