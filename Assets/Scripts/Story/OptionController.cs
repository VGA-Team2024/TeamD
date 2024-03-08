using System;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

namespace Story
{
    public class OptionController : MonoBehaviour
    {
        [SerializeField] private List<Button> _dialogueGameObjects;
        [SerializeField] private List<TextMeshProUGUI> _texts;

        private void Start()
        {
            ResetOption();
        }

        /// <summary>
        /// 登録されたボタンの表示とテキスト更新を行うメソッド
        /// </summary>
        /// <param name="showCount">表示するボタンの数</param>
        /// <param name="texts">表示するテキスト</param>
        public void RegisterOption(List<StoryOption> storyOptions, Action callback)
        {
            _dialogueGameObjects.ForEach(btn=>btn.onClick.RemoveAllListeners());
            for (var i = 0; i < storyOptions.Count; i++)
            {
                _dialogueGameObjects[i].gameObject.SetActive(true);
                var storyOption = storyOptions[i];
                _texts[i].text = storyOption.OptionText;
                _dialogueGameObjects[i].onClick.AddListener(() =>
                {
                    StoryTextManager.Instance.StoryFlag |= storyOption.StoryFlagEnum;
                    storyOption.EventClip?.StartEvent();
                    callback?.Invoke();
                });
            }
        }

        public void ResetOption()
        {
            foreach (var obj in _dialogueGameObjects)
            {
                obj.gameObject.SetActive(false);
            }
        }
    }
}