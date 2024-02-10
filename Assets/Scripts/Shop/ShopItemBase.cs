using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public abstract class ShopItemBase : MonoBehaviour
{
    [SerializeField] private string _name;
    [SerializeField] protected LargeNumber _price;

    public abstract void Buy();
}
