using System;
using System.Collections;
using TMPro;
using UnityEngine;
using UnityEngine.UI;
using TeamD.Enum;

public class ArchivementsUI : MonoBehaviour
{
    [SerializeField] Canvas _parentCanvas;
    [SerializeField] TextMeshProUGUI _textMeshPro;
    [SerializeField] short _waitTime = 1; //テキストを画面内に表示させる時間
    [SerializeField] DataSO _dataSO;
    private Coroutine _coroutine;
    int ScreenHeight;
    string _archiveName = "Archive";

    private void Start()
    {
        float _textHieght = _textMeshPro.preferredHeight;
        ScreenHeight = Screen.currentResolution.height;
        _textMeshPro.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -ScreenHeight / 2 - _textHieght);
    }
    /// <summary>
    /// 画面下に実績獲得のUIを出す。
    /// </summary>
    public void GetArchivements(Achievement achievement)
    {
        if (_coroutine == null)
        {
            _coroutine = StartCoroutine(InatantiateArchivements(achievement));
        }
    }

    IEnumerator InatantiateArchivements(Achievement achievement)
    {
        _archiveName = _dataSO.GetItem(achievement).name;
        _textMeshPro.SetText(_archiveName + "の実績を解除した！");

        float _textHieght = _textMeshPro.preferredHeight;
        _textMeshPro.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -ScreenHeight / 2 + _textHieght);
        yield return new WaitForSeconds(_waitTime);
        _textMeshPro.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -ScreenHeight / 2 - _textHieght);
        _coroutine = null;
    }

    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space)) 
        {
            GetArchivements(Achievement.Lucky);
        }
    }
}
