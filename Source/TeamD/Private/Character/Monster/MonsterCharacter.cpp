#include "Character/Monster/MonsterCharacter.h"
#include "GAS/Monster/MonsterAttributeSet.h"

AMonsterCharacter::AMonsterCharacter()
{
	CharacterAttributeSet = CreateDefaultSubobject<UMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
}

FMonsterBodyPart* AMonsterCharacter::ConvertBoneNameToPart(const FName& TargetBoneName)
{
	for (int i = 0; i < BodyParts.Num(); i++)
	{
		TArray<FString> BoneNameArray;
		BodyParts[i].BoneNames.ToString().ParseIntoArrayLines(BoneNameArray);

		// その部位のボーン名をループ
		for (const FString BoneName : BoneNameArray)
		{
			if (BoneName == TargetBoneName)
			{
				// 同じなら出力終了
				UE_LOG(LogTemp, Log, TEXT("部位名 : %s"), *BodyParts[i].PartName.ToString());
				return &BodyParts[i];
			}
		}
	}

	// どこにも当てはまらなかったら最初を返す
	UE_LOG(LogTemp, Log, TEXT("設定されてないボーン名が入力されましたので先頭の要素 : %s, ボーン名 : %s"), *BodyParts[0].PartName.ToString(), *TargetBoneName.ToString());
	return &BodyParts[0];
}
