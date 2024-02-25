using System.Collections.Generic;
using UnityEngine;

[ExcelAsset(AssetPath = "Resources/Excel")]
public class ReincarnationData : ScriptableObject
{
	public List<ReincarnationDataEntity> Entities;
}
