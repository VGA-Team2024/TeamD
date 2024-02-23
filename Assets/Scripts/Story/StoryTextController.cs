using System;
using System.Collections.Generic;
using Cysharp.Threading.Tasks;
using DG.Tweening;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

namespace Story
{
    public class StoryTextController : MonoBehaviour, IPointerClickHandler
    {
        [SerializeField] private Text _storyTextField;
        [SerializeField] private Text _nameTextField;
        [SerializeField] private float _textShowSpeed;
        [SerializeField, TextArea] private List<string> _stories;
        private Queue<string> _sentence = new();

        public void OnPointerClick(PointerEventData eventData)
        {
            Debug.Log("TextAreaClick");
        }

        public async UniTask UpdateText(int storyNum, string speakerName)
        {
            _nameTextField.text = speakerName;
            foreach (var item in _stories[storyNum].Split())
            {
                _sentence.Enqueue(item);
            }

            for (var i = 0; i < _sentence.Count; i++)
            {
                await _storyTextField.DOText(_sentence.Dequeue(), _textShowSpeed)
                    .SetEase(Ease.Linear)
                    .AsyncWaitForCompletion();
            }
        }

        public async UniTask TestUpdateText(string speakerName, List<string> texts)
        {
            _nameTextField.text = speakerName;
            foreach (var item in texts)
            {
                _sentence.Enqueue(item);
            }

            for (var i = 0; i < texts.Count; i++)
            {
                await _storyTextField.DOText(_sentence.Dequeue(), _textShowSpeed)
                    .SetEase(Ease.Linear)
                    .AsyncWaitForCompletion();
                await UniTask.Delay(TimeSpan.FromSeconds(2));
                _storyTextField.text = "";
            }
        }
    }
}