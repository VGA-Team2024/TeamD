using TMPro;
using UnityEngine;

public class ResourcesTextController : MonoBehaviour
{
    private TextMeshProUGUI _text;
    private PlayerManager _playerManager = new();

    private void Start()
    {
        _text = GetComponent<TextMeshProUGUI>();
    }

    /// <summary>
    /// UnityEvent登録用テキスト更新メソッド
    /// </summary>
    public void UpdateText()
    {
        _text.text = _playerManager.PlayerResources.ToString("F2");
    }
}