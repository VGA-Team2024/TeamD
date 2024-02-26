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
            ResetOption();
        }

        /// <summary>
        /// 登録されたボタンの表示とテキスト更新を行うメソッド
        /// </summary>
        /// <param name="showCount">表示するボタンの数</param>
        /// <param name="texts">表示するテキスト</param>
        public void UpdateOption(int showCount, List<string> texts)
        {
            Debug.Log(showCount);
            for (var i = 0; i < showCount; i++)
            {
                _dialogueGameObjects[i].SetActive(true);
                _texts[i].text = texts[i];
            }
            
        }

        public void ResetOption()
        {
            foreach (var obj in _dialogueGameObjects)
            {
                obj.SetActive(false);
            }
        }
    }
}