using System;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

/// <summary>
/// ショップのアイテムボタン
/// </summary>
public class ShopItemButton : MonoBehaviour, IPointerClickHandler
{
    [SerializeField] public Text _itemName;
    public event Action OnClickEvent;
    
    public void OnPointerClick(PointerEventData eventData)
    {
        Debug.Log("クリック！");
        OnClickEvent?.Invoke();
    }

    public void SetItemName(string name)
    {
        _itemName.text = name;
    }
}
