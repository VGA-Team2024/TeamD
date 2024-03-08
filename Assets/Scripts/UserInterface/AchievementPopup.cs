using TMPro;
using UnityEngine;
using UnityEngine.UI;
/// <summary>
/// 実績解除時に出現させるUI
/// </summary>
public class AchievementPopup : MonoBehaviour
{
    [SerializeField] Button _closeButton;
    [SerializeField] TextMeshProUGUI _title;
    public TextMeshProUGUI Title => _title;

    void Awake()
    {
        if(_closeButton) _closeButton.onClick.AddListener(()=>Destroy(gameObject));
    }
}
