using System;
using System.Collections.Generic;
using System.Threading;
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

        public void OnPointerClick(PointerEventData eventData)
        {
            Debug.Log("TextAreaClick");
        }

        /// <summary>
        /// メインテキストを文字送りするメソッド
        /// </summary>
        /// <param name="speakerName">話者名</param>
        /// <param name="texts">表示するテキスト一覧</param>
        /// <param name="cts">キャンセルトークン</param>
        public async UniTask UpdateTextAsync(string speakerName, List<string> texts , CancellationTokenSource cts)
        {
            cts.Cancel();
            _nameTextField.text = speakerName;

            foreach (var item in texts)
            {
                await _storyTextField.DOText(item, _textShowSpeed)
                    .SetEase(Ease.Linear)
                    .AsyncWaitForCompletion();
                await UniTask.Delay(TimeSpan.FromSeconds(2));
                _storyTextField.text = "";
            }
        }
    }
}