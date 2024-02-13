using System;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

/// <summary>
/// ショップのアイテムボタン
/// </summary>
public class ShopItemButton : MonoBehaviour, IPointerClickHandler
{
    [SerializeField] private Text _itemName;
    [SerializeField] private Text _currentOwnNumText;
    public event Action OnClickEvent;
    private int _currentOwnNum = 0;
    private bool _isFactory = false;
    
    public void OnPointerClick(PointerEventData eventData)
    {
        Debug.Log("クリック！");
        OnClickEvent?.Invoke();
    }

    /// <summary>
    /// 施設ならtrueを渡す
    /// </summary>
    /// <param name="name"></param>
    /// <param name="isFactory"></param>
    public void SetItemName(string name, bool isFactory)
    {
        _itemName.text = name;
        _isFactory = isFactory;
    }

    public void SetCurrentOwnNum()
    {
        if (!_isFactory) return;
        _currentOwnNum++;
        _currentOwnNumText.text = _currentOwnNum.ToString();
    }
}
