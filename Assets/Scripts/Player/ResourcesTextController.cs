using TMPro;
using UnityEngine;

public class ResourcesTextController : MonoBehaviour
{
    private TextMeshProUGUI _text;
    private PlayerManager _playerManager = PlayerManager.Instance;

    private void Start()
    {
        _text = GetComponent<TextMeshProUGUI>();
    }

    private void Update()
    {
        UpdateText();
    }

    /// <summary>
    /// UnityEvent登録用テキスト更新メソッド
    /// </summary>
    public void UpdateText()
    {
        _text.text = _playerManager.CookieCount.ToString();
    }
}