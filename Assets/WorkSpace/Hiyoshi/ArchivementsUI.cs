using System.Collections;
using TMPro;
using UnityEngine;
using TeamD.Enum;

public class ArchivementsUI : MonoBehaviour
{
    [SerializeField] Canvas _parentCanvas;
    [SerializeField] TextMeshProUGUI _textMeshPro;
    [SerializeField] short _waitTime = 1; //テキストを画面内に表示させる時間
    [SerializeField] DataSO _dataSO;
    private Coroutine _coroutine;
    int _screenHeight;

    private void Start()
    {
        float _textHieght = _textMeshPro.preferredHeight;
        _screenHeight = Screen.currentResolution.height;
        _textMeshPro.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -_screenHeight / 2 - _textHieght);
    }
    /// <summary>
    /// 画面下に実績獲得のUIを出す。
    /// </summary>
    public void GetAchievements(Achievement achievement)
    {
        if (_coroutine == null)
        {
            _coroutine = StartCoroutine(InstantiateAchievements(achievement));
        }
    }

    IEnumerator InstantiateAchievements(Achievement achievement)
    {
        _textMeshPro.SetText(_dataSO.GetItem(achievement).name + "の実績を解除した！");
        float _textHieght = _textMeshPro.preferredHeight;
        _textMeshPro.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -_screenHeight / 2 + _textHieght);
        yield return new WaitForSeconds(_waitTime);
        _textMeshPro.GetComponent<RectTransform>().anchoredPosition = new Vector2(0, -_screenHeight / 2 - _textHieght);
        _coroutine = null;
    }
}
