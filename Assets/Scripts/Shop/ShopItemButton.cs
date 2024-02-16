using System;
using System.Globalization;
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
    private bool _isFactory;
    public event Action OnLeftClickEvent;
    public event Action OnRightClickEvent;

    public int CurrentOwnNum { get; private set; }

    public void OnPointerClick(PointerEventData eventData)
    {
        if (eventData.button == PointerEventData.InputButton.Left)
        {
            OnLeftClickEvent?.Invoke();
        }
        else if (eventData.button == PointerEventData.InputButton.Right)
        {
            OnRightClickEvent?.Invoke();
        }
    }

    /// <summary>
    /// 施設ならtrueを渡す
    /// </summary>
    public void SetItemName(string itemName, bool isFactory)
    {
        _itemName.text = itemName;
        _isFactory = isFactory;
        if (!_isFactory)
        {
            _currentOwnNumText.gameObject.SetActive(false);
        }
    }

    public void SetCurrentOwnNum(int addValue)
    {
        if (!_isFactory) return;
        CurrentOwnNum += addValue;
        _currentOwnNumText.text = CurrentOwnNum.ToString();
    }

    public void SetPriceText(double price)
    {
        _priceText.text = price.ToString(CultureInfo.CurrentCulture);
    }
}
