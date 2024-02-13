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
    [SerializeField] private Text _priceText;
    private int _currentOwnNum = 0;
    private bool _isFactory = false;
    public event Action OnClickEvent;
    
    public void OnPointerClick(PointerEventData eventData)
    {
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
        if (!_isFactory)
        {
            _currentOwnNumText.gameObject.SetActive(false);
        }
    }

    public void SetCurrentOwnNum()
    {
        if (!_isFactory) return;
        _currentOwnNum++;
        _currentOwnNumText.text = _currentOwnNum.ToString();
    }

    public void SetPrice(LargeNumber price)
    {
        _priceText.text = price.ToString();
    }
}
