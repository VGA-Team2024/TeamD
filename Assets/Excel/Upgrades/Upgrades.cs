using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExcelAsset(AssetPath = "Resources/Excel")]
public class Upgrades : ScriptableObject
{
	public List<UpgradesEntity> Entities;
}
