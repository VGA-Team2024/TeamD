using System.Collections.Generic;
using UnityEngine;

[ExcelAsset(AssetPath = "Resources/Excel")]
public class Achievements : ScriptableObject
{
	public List<AchievementsEntity> Entities;
}
