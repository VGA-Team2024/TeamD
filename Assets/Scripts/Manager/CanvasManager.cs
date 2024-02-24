using UnityEngine;
using UnityEngine.UI;

/// <summary>
/// CanvasやCanvas周りのオブジェクトをまとめるクラス
/// </summary>
public class CanvasManager : MonoBehaviour
{
    [SerializeField] Button _reincarnationButton;
    [SerializeField] Button _storyButton;
    [SerializeField, Tooltip("実績解放時にポップアップを出す場所")] RectTransform _achievementPopupParent;

    [SerializeField, Tooltip("ポップアップのプレハブ")]
    AchievementPopup _achievementPopup;
    public Button ReincarnationButton => _reincarnationButton;
    public Button StoryButton => _storyButton;
    public RectTransform AchievementPopupParent => _achievementPopupParent;
    public AchievementPopup AchievementPopup => _achievementPopup;

    void Awake()
    {
        _reincarnationButton.interactable = false;
        _storyButton.interactable = false;
    }
}
