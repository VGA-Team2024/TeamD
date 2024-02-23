using UnityEngine;
using UnityEngine.UI;

/// <summary>
/// メニュー管理。UIをInstantiateするための親オブジェクトを持っている。
/// </summary>
public class Menu : MonoBehaviour
{
    [SerializeField] Button _reincarnationButton;
    [SerializeField] Button _storyButton;
    public Button ReincarnationButton => _reincarnationButton;
    public Button StoryButton => _storyButton;

    void Awake()
    {
        _reincarnationButton.interactable = false;
        _storyButton.interactable = false;
    }
}
