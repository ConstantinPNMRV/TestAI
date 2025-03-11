// Fill out your copyright notice in the Description page of Project Settings.

#include "Bot.h"
#include "AIControllerCustom.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/NavMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ABot::ABot()
{
	PrimaryActorTick.bCanEverTick = true;

	// Инициализация SkeletalMesh
	USkeletalMeshComponent* SkeletalMeshComponent = GetMesh();
	SkeletalMeshComponent->SetRelativeLocation(FVector(0, 0, -89));
	SkeletalMeshComponent->SetRelativeRotation(FRotator(0, 270, 0));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BotSkeletalMesh(TEXT("SkeletalMesh'/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple'"));
	if (BotSkeletalMesh.Succeeded())
		SkeletalMeshComponent->SetSkeletalMesh(BotSkeletalMesh.Object);

	// Задание AnimBlueprint для SkeletalMesh
	static ConstructorHelpers::FObjectFinder<UAnimBlueprint> BotAnimBlueprint(TEXT("AnimBlueprint'/Game/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny'"));
	if (BotAnimBlueprint.Succeeded())
		SkeletalMeshComponent->SetAnimClass(BotAnimBlueprint.Object->GeneratedClass);

	// Инициализация BehaviorTree
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BotBehaviorTree(TEXT("BehaviorTree'/Game/Blueprints/BT_BotCharacter.BT_BotCharacter'"));
	if (BotBehaviorTree.Succeeded())
		BehaviorTree = BotBehaviorTree.Object;

	// Перезапись класса AIController по умолчанию
	static ConstructorHelpers::FClassFinder<AAIControllerCustom> BotAIControllerCustom(TEXT("/Script/CoreUObject.Class'/Script/TestAI.AIControllerCustom'"));
	if (BotAIControllerCustom.Succeeded())
		AIControllerClass = BotAIControllerCustom.Class;

	// Подписываемся на OnComponentHit от капсулы
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &ABot::BindOnComponentHit);

	// Инициализация компонента для толпы подобных ботов
	CrowdFollowingComponent = CreateDefaultSubobject<UCrowdFollowingComponent>("CrowdFollowingComponent");

	// Задание плавного разворота у бота
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void ABot::BindOnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (UKismetSystemLibrary::GetClassDisplayName(OtherActor->GetClass()) == "BP_FirstPersonProjectile_C")
	{
		HitCounter++;
		if (HitCounter == 4)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::FromHex("00D4FFFF"), FString::Printf(TEXT("Enemy is Dead")));
			UE_LOG(LogTemp, Display, TEXT("Enemy is Dead"));
			SetEnableBehaviorTree(false);
		}
		else if (HitCounter < 4)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5, FColor::FromHex("00D4FFFF"), FString::Printf(TEXT("-25 hp")));
			UE_LOG(LogTemp, Display, TEXT("-25 hp"));
		}
	}
}

UBehaviorTree* ABot::GetBehaviorTree() const
{
	return BehaviorTree;
}

void ABot::SetEnableBehaviorTree(bool IsOn)
{
	AAIControllerCustom* AIControllerCustom = Cast<AAIControllerCustom>(GetController());
	AIControllerCustom->SetEnableBehaviorTree(IsOn);

	if (IsOn)
		HitCounter = 0;
}
