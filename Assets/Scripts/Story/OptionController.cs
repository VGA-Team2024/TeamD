using System.Collections.Generic;
using TMPro;
using UnityEngine;

namespace Story
{
    public class OptionController : MonoBehaviour
    {
        [SerializeField] private List<GameObject> _dialogueGameObjects;
        [SerializeField] private List<TextMeshProUGUI> _texts;

        private void Start()
        {
            foreach (var obj in _dialogueGameObjects)
            {
                obj.SetActive(false);
            }
        }

        /// <summary>
        /// 登録されたボタンの表示とテキスト更新を行うメソッド
        /// </summary>
        /// <param name="showCount">表示するボタンの数</param>
        /// <param name="texts">表示するテキスト</param>
        public void UpdateDialogue(int showCount, List<string> texts)
        {
            for (var i = 0; i < showCount; i++)
            {
                _dialogueGameObjects[i].SetActive(true);
                _texts[i].text = texts[i];
            }
            
        }
    }
}