using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExcelAsset(AssetPath = "Resources/Excel")]
public class Tiers : ScriptableObject
{
	public List<TiersEntity> Entities;
}
