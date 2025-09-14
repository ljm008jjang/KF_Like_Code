// Fill out your copyright notice in the Description page of Project Settings.

#if WITH_EDITOR
#include "MonsterBlendSpaceGenerator.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/BlendSpaceFactoryNew.h"
#include "KillingFloorLike/Monster.h"
#include "UObject/SavePackage.h"

class FAssetRegistryModule;

UMonsterBlendSpaceGenerator::UMonsterBlendSpaceGenerator()
{
}



void UMonsterBlendSpaceGenerator::GenerateBlendSpace2D()
{
	// 3. 에셋 레지스트리에서 애니메이션 시퀀스 로드

	// 4. BlendSpace 생성 및 초기화 반복
	for (EMonsterType MonsterType : TEnumRange<EMonsterType>())
	{
		if (MonsterType == EMonsterType::None)
		{
			continue;
		}

		const UEnum* EnumPtr = StaticEnum<EMonsterType>();
		if (!EnumPtr) continue; // UEnum을 찾지 못하면 종료

		// 2. enum 값(정수형)에 해당하는 짧은 이름을 FString으로 직접 가져옵니다.
		FString MonsterTypeString = EnumPtr->GetNameStringByValue((int64)MonsterType);
		FString TargetMonsterAssetPath = AssetPath + "/" + MonsterTypeString;
		TArray<FAssetData> AssetDataList = GetAnimationAssets(TargetMonsterAssetPath);

		//TArray<FAssetData> SelectedAssets = FilterAssetsByAnimationName(AssetDataList, Value.animation_name);

		for (int i = 0; i < 2; i++)
		{
			const bool IsHeadless = (i == 1);
			FString NewAssetName = IsHeadless
				                       ? FString::Printf(TEXT("BS_%s_Headless_Locomotion"), *MonsterTypeString)
				                       : FString::Printf(TEXT("BS_%s_Locomotion"), *MonsterTypeString);

			FString FullPackageName = TargetMonsterAssetPath / NewAssetName;

			UBlendSpace* BlendSpace = CreateOrFindBlendSpace(TargetMonsterAssetPath, NewAssetName,
			                                                 *(TargetMonsterAssetPath / MonsterTypeString +
				                                                 "_Freak_Skeleton"));
			if (!BlendSpace)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create or find BlendSpace: %s"),
				       *(FullPackageName));
				continue;
			}


			//고려할게 너무 많아진다... 그냥 제작하는데 더 편할듯
			/*// BlendParameters 설정
			SetupBlendParameters(BlendSpace);

			// 샘플 추가
			AddSamplesToBlendSpace(BlendSpace, SelectedAssets, IsHeadless);

			// BlendSpace 초기화 및 저장
			FinalizeAndSaveBlendSpace(BlendSpace, FullPackageName);*/
		}
	}
}

// ---------------------------
// 각 기능별 분리 함수 예시

TArray<FAssetData> UMonsterBlendSpaceGenerator::GetAnimationAssets(const FString& Path)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.PackagePaths.Add(*Path);
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);
	return AssetDataList;
}

TArray<FAssetData> UMonsterBlendSpaceGenerator::FilterAssetsByAnimationName(const TArray<FAssetData>& Assets,
                                                                            const FString& AnimationName)
{
	return Assets.FilterByPredicate([AnimationName](const FAssetData& AssetData)
	{
		FString Name = AssetData.AssetName.ToString();
		TArray<FString> Tokens;
		Name.ParseIntoArray(Tokens, TEXT("_"));
		return AnimationName.Equals(Tokens.Last());
	});
}

UBlendSpace* UMonsterBlendSpaceGenerator::CreateOrFindBlendSpace(const FString& TargetMonsterAssetPath,
                                                                 const FString& AssetName,
                                                                 const FString& SkeletalAssetPath)
{
	const FString FullPackagePath = FPaths::Combine(TargetMonsterAssetPath, AssetName);
	UPackage* Package = CreatePackage(*FullPackagePath);
	if (!Package) return nullptr;

	UBlendSpace* BlendSpace = FindObject<UBlendSpace>(Package, *AssetName);
	if (!BlendSpace)
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UBlendSpaceFactoryNew* Factory = NewObject<UBlendSpaceFactoryNew>();
		// 1. Skeleton 로드
		USkeleton* TargetSkeleton = LoadObject<USkeleton>(
			nullptr,

			*SkeletalAssetPath);
		if (!TargetSkeleton)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load Skeleton : %s"), *SkeletalAssetPath);
			return nullptr;
		}
		Factory->TargetSkeleton = TargetSkeleton;

		BlendSpace = Cast<UBlendSpace>(
			AssetTools.CreateAsset(AssetName, TargetMonsterAssetPath, UBlendSpace::StaticClass(), Factory));
	}
	return BlendSpace;
}

void UMonsterBlendSpaceGenerator::SetupBlendParameters(UBlendSpace* BlendSpace)
{
	FProperty* Prop = BlendSpace->GetClass()->FindPropertyByName(FName("BlendParameters"));
	if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
	{
		UScriptStruct* StructType = StructProp->Struct;
		void* ArrayPtr = StructProp->ContainerPtrToValuePtr<void>(BlendSpace);

		// X축 (Angle)
		void* ElementPtr = (uint8*)ArrayPtr + 0 * StructType->GetStructureSize();
		FBlendParameter* Param = reinterpret_cast<FBlendParameter*>(ElementPtr);
		Param->DisplayName = "Angle";
		Param->Min = -180;
		Param->Max = 180;
		Param->GridNum = 4;
		Param->bSnapToGrid = true;

		// Y축 (Speed)
		ElementPtr = (uint8*)ArrayPtr + 1 * StructType->GetStructureSize();
		Param = reinterpret_cast<FBlendParameter*>(ElementPtr);
		Param->DisplayName = "Speed";
		Param->Min = -100;
		Param->Max = 100;
		Param->GridNum = 4;
		Param->bSnapToGrid = true;
	}
}

void UMonsterBlendSpaceGenerator::AddSamplesToBlendSpace(UBlendSpace* BlendSpace, const TArray<FAssetData>& Assets,
                                                         bool bIsHeadless)
{
	for (const FAssetData& AssetData : Assets)
	{
		UAnimSequence* Anim = Cast<UAnimSequence>(AssetData.GetAsset());
		if (!Anim) continue;

		FString Name = AssetData.AssetName.ToString();
		/*TArray<FString> Tokens;
		Name.ParseIntoArray(Tokens, TEXT("_"));
		if (Tokens.Num() <= 0) continue;

		FString Prefix = Tokens[0];*/
		if (!bIsHeadless)
		{
			AddSamplesForBasicPrefix(BlendSpace, Name, Anim);
		}
		else
		{
			AddSamplesForHeadlessPrefix(BlendSpace, Name, Anim);
		}
	}
}

void UMonsterBlendSpaceGenerator::AddSamplesForBasicPrefix(UBlendSpace* BlendSpace, const FString& AssetName,
                                                           UAnimSequence* Anim)
{
	if (AssetName == "Idle")
	{
		TArray<FVector> Positions = {FVector(0, 0, 0), FVector(-180, 0, 0), FVector(180, 0, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (AssetName == "JogB")
	{
		TArray<FVector> Positions = {FVector(0, -100, 0), FVector(180, -100, 0), FVector(-180, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (AssetName == "JogF")
	{
		TArray<FVector> Positions = {FVector(0, 100, 0), FVector(180, 100, 0), FVector(-180, 100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (AssetName == "JogL")
	{
		TArray<FVector> Positions = {FVector(-90, 100, 0), FVector(-90, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (AssetName == "JogR")
	{
		TArray<FVector> Positions = {FVector(90, 100, 0), FVector(90, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (AssetName == "TurnL")
	{
		BlendSpace->AddSample(Anim, FVector(-90, 0, 0));
	}
	else if (AssetName == "TurnR")
	{
		BlendSpace->AddSample(Anim, FVector(90, 0, 0));
	}
}

void UMonsterBlendSpaceGenerator::AddSamplesForHeadlessPrefix(UBlendSpace* BlendSpace, const FString& Prefix,
                                                         UAnimSequence* Anim)
{
	if (Prefix == "CHIdle")
	{
		TArray<FVector> Positions = {FVector(0, 0, 0), FVector(-180, 0, 0), FVector(180, 0, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "CHWalkB")
	{
		TArray<FVector> Positions = {FVector(0, -100, 0), FVector(180, -100, 0), FVector(-180, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "CHWalkF")
	{
		TArray<FVector> Positions = {FVector(0, 100, 0), FVector(180, 100, 0), FVector(-180, 100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "CHWalkL")
	{
		TArray<FVector> Positions = {FVector(-90, 100, 0), FVector(-90, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "CHWalkR")
	{
		TArray<FVector> Positions = {FVector(90, 100, 0), FVector(90, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
}

void UMonsterBlendSpaceGenerator::FinalizeAndSaveBlendSpace(UBlendSpace* BlendSpace, const FString& FullPackageName)
{
	FAssetRegistryModule::AssetCreated(BlendSpace);

	//중요!
	BlendSpace->ResampleData();
	BlendSpace->ValidateSampleData();

	BlendSpace->Modify();
	BlendSpace->PostEditChange();
	BlendSpace->MarkPackageDirty();

	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		*FullPackageName, FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

	bool bSaved = UPackage::SavePackage(BlendSpace->GetOutermost(), BlendSpace, *PackageFileName, SaveArgs);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save BlendSpace package: %s"), *FullPackageName);
	}
}


/*void UMonsterBlendSpaceGenerator::GenerateBlendSpace2D()
{
	UBlendSpaceFactoryNew* Factory = NewObject<UBlendSpaceFactoryNew>();
	Factory->TargetSkeleton = LoadObject<USkeleton>(
		nullptr, TEXT(
			"/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/SkeletalMeshes/British_Soldier1_Skeleton.British_Soldier1_Skeleton"));

	if (WeaponDataMap.IsEmpty())
	{
		SetWeaponData();
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataList;
	FARFilter Filter;
	Filter.PackagePaths.Add(*AssetPath);
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);

	TArray<UPackage*> PackagesToSave;
	TArray<UObject*> Objects;


	for (auto& [Key, Value] : WeaponDataMap)
	{
		TArray<FAssetData> SelectedAssetDataList = AssetDataList.FilterByPredicate([Value](const FAssetData& AssetData)
		{
			FString Name = AssetData.AssetName.ToString();
			TArray<FString> Tokens;
			Name.ParseIntoArray(Tokens, TEXT("_"));
			return Value.animation_name.Equals(Tokens.Last());
		});

		for (int i = 0; i < 2; i++)
		{
			FString NewAssetName;
			if (i == 0)
			{
				NewAssetName = "BS_" + Value.animation_name;
			}
			else
			{
				NewAssetName = "BS_CH_" + Value.animation_name;
			}
			FString FullPackageName = BlendPath + "/" + NewAssetName;

			// 에셋이 이미 존재하는지 확인
			/*if (StaticLoadObject(UObject::StaticClass(), nullptr, *FullPackageName, nullptr, LOAD_NoWarn, nullptr))
			{
				UE_LOG(LogTemp, Warning, TEXT("Asset already exists, skipping: %s"), *FullPackageName);
				continue;
			}#1#

			UPackage* Package = CreatePackage(*FullPackageName);
			if (!Package) continue;

			bool bIsNewlyCreated = false;
			UBlendSpace* BlendSpace = FindObject<UBlendSpace>(Package, *NewAssetName);
			if (!BlendSpace)
			{
				IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
				//UBlendSpaceFactoryNew* Factory = NewObject<UBlendSpaceFactoryNew>();
				// Factory->TargetSkeleton은 샘플 애니메이션의 Skeleton으로 뒤에서 설정/검증됩니다.

				UObject* Created = AssetTools.CreateAsset(NewAssetName, BlendPath, UBlendSpace::StaticClass(),
				                                          Factory);
				BlendSpace = Cast<UBlendSpace>(Created);
				bIsNewlyCreated = (BlendSpace != nullptr);
			}

			FProperty* Prop = BlendSpace->GetClass()->FindPropertyByName(FName("BlendParameters"));
			if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
			{
				UScriptStruct* StructType = StructProp->Struct;
				void* ArrayPtr = StructProp->ContainerPtrToValuePtr<void>(BlendSpace);


				//가로
				void* ElementPtr = (uint8*)ArrayPtr + 0 * StructType->GetStructureSize();
				FBlendParameter* Param = reinterpret_cast<FBlendParameter*>(ElementPtr);

				Param->DisplayName = "Angle";
				Param->Min = -180;
				Param->Max = 180;
				Param->GridNum = 4;
				Param->bSnapToGrid = true;

				//세로
				ElementPtr = (uint8*)ArrayPtr + 1 * StructType->GetStructureSize();
				Param = reinterpret_cast<FBlendParameter*>(ElementPtr);

				Param->DisplayName = "Speed";
				Param->Min = -100;
				Param->Max = 100;
				Param->GridNum = 4;
				Param->bSnapToGrid = true;

				for (FAssetData& AssetData : SelectedAssetDataList)
				{
					UAnimSequence* Anim = Cast<UAnimSequence>(AssetData.GetAsset());
					if (!Anim) continue;

					FString Name = AssetData.AssetName.ToString();
					TArray<FString> Tokens;
					Name.ParseIntoArray(Tokens, TEXT("_"));

					if (Tokens.Num() <= 0) continue;

					FVector SamplePosition;
					FString Prefix = Tokens[0];

					if (i == 0)
					{
						if (Prefix == "Idle")
						{
							TArray<FVector> Positions = {
								FVector(0, 0, 0), FVector(-180, 0, 0), FVector(180, 0, 0)
							};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "JogB")
						{
							TArray<FVector> Positions = {
								FVector(0, -100, 0), FVector(180, -100, 0), FVector(-180, -100, 0)
							};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "JogF")
						{
							TArray<FVector> Positions = {
								FVector(0, 100, 0), FVector(180, 100, 0), FVector(-180, 100, 0)
							};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "JogL")
						{
							TArray<FVector> Positions = {FVector(-90, 100, 0), FVector(-90, -100, 0)};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "JogR")
						{
							TArray<FVector> Positions = {FVector(90, 100, 0), FVector(90, -100, 0)};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "TurnL" && i == 0)
						{
							SamplePosition = FVector(-90, 0, 0);
							BlendSpace->AddSample(Anim, SamplePosition);
						}
						else if (Prefix == "TurnR" && i == 0)
						{
							SamplePosition = FVector(90, 0, 0);
							BlendSpace->AddSample(Anim, SamplePosition);
						}
					}
					else
					{
						if (Prefix == "CHIdle")
						{
							TArray<FVector> Positions = {
								FVector(0, 0, 0), FVector(-180, 0, 0), FVector(180, 0, 0)
							};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "CHWalkB")
						{
							TArray<FVector> Positions = {
								FVector(0, -100, 0), FVector(180, -100, 0), FVector(-180, -100, 0)
							};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "CHWalkF")
						{
							TArray<FVector> Positions = {
								FVector(0, 100, 0), FVector(180, 100, 0), FVector(-180, 100, 0)
							};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "CHWalkL")
						{
							TArray<FVector> Positions = {FVector(-90, 100, 0), FVector(-90, -100, 0)};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
						else if (Prefix == "CHWalkR")
						{
							TArray<FVector> Positions = {FVector(90, 100, 0), FVector(90, -100, 0)};
							for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
						}
					}
				}
			}
			FAssetRegistryModule::AssetCreated(BlendSpace);

			//이걸 추가하니 문제가 해결됨...
			BlendSpace->ResampleData();
			BlendSpace->ValidateSampleData();

			BlendSpace->Modify();
			BlendSpace->PostEditChange(); // 에디터 변경 반영(내부 재계산 유도)
			BlendSpace->MarkPackageDirty();

			FString PackageFileName = FPackageName::LongPackageNameToFilename(
				*FullPackageName, FPackageName::GetAssetPackageExtension());
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

			bool bSaved = UPackage::SavePackage(Package, BlendSpace, *PackageFileName, SaveArgs);
			if (!bSaved)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to save BlendSpace package"));
			}
		}
	}
	
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		"ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(Objects);

}*/
#endif
