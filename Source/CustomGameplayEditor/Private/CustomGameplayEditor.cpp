#include "CustomGameplayEditor.h"

#include "BlendSpaceGenerator.h"
#include "MonsterBlendSpaceGenerator.h"
#include "WeaponAnimationAssigner.h"
#include "WeaponMontageGenerator.h"
//#include "IConsoleManager.h" // 이 줄을 추가하세요.

#define LOCTEXT_NAMESPACE "FCustomGameplayEditorModule"

void FCustomGameplayEditorModule::StartupModule()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("GenerateBlendSpace"),
		TEXT("Generates a BlendSpace2D asset."),
		FConsoleCommandWithArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args)
			{
				// UObject로 생성하고, 가비지 컬렉션 방지를 위해 AddToRoot 처리
				UBlendSpaceGenerator* Generator = NewObject<UBlendSpaceGenerator>();
				Generator->AddToRoot();
				Generator->GenerateBlendSpace2D();
				Generator->RemoveFromRoot(); // 작업 완료 후 Root에서 제거
			}
		)
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("AssignWeaponAnimation"),
		TEXT("Assigns a WeaponAnimation asset."),
		FConsoleCommandWithArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args)
			{
				// UObject로 생성하고, 가비지 컬렉션 방지를 위해 AddToRoot 처리
				UWeaponAnimationAssigner* Generator = NewObject<UWeaponAnimationAssigner>();
				Generator->AddToRoot();
				Generator->AssignWeaponAnimations();
				Generator->RemoveFromRoot(); // 작업 완료 후 Root에서 제거
			}
		)
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("GenerateWeaponMontage"),
		TEXT("Generates a WeaponMontage asset."),
		FConsoleCommandWithArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args)
			{
				// UObject로 생성하고, 가비지 컬렉션 방지를 위해 AddToRoot 처리
				UWeaponMontageGenerator* Generator = NewObject<UWeaponMontageGenerator>();
				Generator->AddToRoot();
				Generator->GenerateWeaponMontage();
				Generator->RemoveFromRoot(); // 작업 완료 후 Root에서 제거
			}
		)
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("GenerateMonsterBlendSpace"),
		TEXT("Generates a BlendSpace2D asset."),
		FConsoleCommandWithArgsDelegate::CreateLambda(
			[](const TArray<FString>& Args)
			{
				// UObject로 생성하고, 가비지 컬렉션 방지를 위해 AddToRoot 처리
				UMonsterBlendSpaceGenerator* Generator = NewObject<UMonsterBlendSpaceGenerator>();
				Generator->AddToRoot();
				Generator->GenerateBlendSpace2D();
				Generator->RemoveFromRoot(); // 작업 완료 후 Root에서 제거
			}
		)
	);
}


void FCustomGameplayEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCustomGameplayEditorModule, CustomGameplayEditor)
