// Fill out your copyright notice in the Description page of Project Settings.

#include "AIControllerCustom.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"

AAIControllerCustom::AAIControllerCustom(FObjectInitializer const& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerceptionComponent");
	SetPerceptionComponent(*AIPerceptionComponent);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAIControllerCustom::BindOnTargetPerceptionUpdated);

	// Инициализация зрения бота
	AISense_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISense_Sight"));
	AISense_Sight->SightRadius = 3000;
	AISense_Sight->LoseSightRadius = 3500;
	AISense_Sight->PeripheralVisionAngleDegrees = 70;
	AISense_Sight->DetectionByAffiliation.bDetectEnemies = true;
	AISense_Sight->DetectionByAffiliation.bDetectNeutrals = true;
	AISense_Sight->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComponent->ConfigureSense(*AISense_Sight);
	AIPerceptionComponent->SetDominantSense(AISense_Sight->GetSenseImplementation());

	// Инициализация слуха бота
	AISense_Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("AISense_Hearing"));
	AISense_Hearing->HearingRange = 5000;
	AISense_Hearing->DetectionByAffiliation.bDetectEnemies = true;
	AISense_Hearing->DetectionByAffiliation.bDetectNeutrals = true;
	AISense_Hearing->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerceptionComponent->ConfigureSense(*AISense_Hearing);

	// Инициализация детекции получения урона ботом
	AISense_Damage = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("AISense_Damage"));
	AIPerceptionComponent->ConfigureSense(*AISense_Damage);
}

void AAIControllerCustom::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Bot = Cast<ABot>(InPawn);
	if (Bot != nullptr)
	{
		if (UBehaviorTree* const BehaviorTree = Bot->GetBehaviorTree())
		{
			UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp);
			Blackboard = BlackboardComp;
			RunBehaviorTree(BehaviorTree);

			CalculatePatrolLocations();
			SetEnableBehaviorTree(false);
		}
	}
}

void AAIControllerCustom::BindOnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	PlayerEnemy = Cast<APawn>(Actor);
	StimulusCurrent = Stimulus;

	if (UGameplayStatics::GetPlayerPawn(this, 0) == PlayerEnemy)
	{
		FName SenseName = UAIPerceptionSystem::GetSenseClassForStimulus(this, StimulusCurrent)->GetFName();
		if (SenseName == "AISense_Hearing")
			BotHearingHandler();
		else if (SenseName == "AISense_Sight")
			BotVisionHandler();
		else if (SenseName == "AISense_Damage")
			BotDamageHandler();
	}
}

void AAIControllerCustom::SetEnableBehaviorTree(bool IsOn) const
{
	if (IsOn)
		BrainComponent->StartLogic();
	else
		BrainComponent->StopLogic("Stop");
}

void AAIControllerCustom::SetIsHearNoise(bool IsOn) const
{
	BlackboardComp->SetValueAsBool("IsHearNoise", IsOn);
}

void AAIControllerCustom::SetPatrolLocations(FVector ForwardLocation, FVector BackwardLocation) const
{
	BlackboardComp->SetValueAsVector("ForwardLocation", ForwardLocation);
	BlackboardComp->SetValueAsVector("BackwardLocation", BackwardLocation);
}

void AAIControllerCustom::CalculatePatrolLocations() const
{
	SetPatrolLocations(Bot->GetActorLocation() + (Bot->GetActorForwardVector() * 2000), Bot->GetActorLocation() + (Bot->GetActorForwardVector() * -2000));
}

FVector AAIControllerCustom::CalculateNoiseLocation(FVector StimulusLocation, FVector ReceiverLocation) const
{
	return ReceiverLocation + UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::FindLookAtRotation(ReceiverLocation, StimulusLocation)) * UKismetMathLibrary::RandomFloatInRange(100, 1000);
}

void AAIControllerCustom::BotVisionHandler()
{
	SetIsHearNoise(false);

	IsHasPlayer = StimulusCurrent.WasSuccessfullySensed();

	BlackboardComp->SetValueAsBool("IsHasPlayer", IsHasPlayer);

	if (IsHasPlayer)
	{
		BlackboardComp->SetValueAsObject("Player", PlayerEnemy);
	}
	else
	{
		BlackboardComp->ClearValue("Player");
		SetIsHearNoise(true);
		BlackboardComp->SetValueAsVector("NoiseLocation", StimulusCurrent.StimulusLocation);
	}
}

void AAIControllerCustom::BotHearingHandler()
{
	if (!IsHasPlayer)
	{
		SetIsHearNoise(false);
		if (StimulusCurrent.WasSuccessfullySensed())
		{
			SetIsHearNoise(true);
			BlackboardComp->SetValueAsVector("NoiseLocation", CalculateNoiseLocation(StimulusCurrent.StimulusLocation, StimulusCurrent.ReceiverLocation));
		}
	}
}

void AAIControllerCustom::BotDamageHandler()
{
	BlackboardComp->SetValueAsObject("Player", PlayerEnemy);
	SetIsHearNoise(false);
	IsHasPlayer = true;
	BlackboardComp->SetValueAsBool("IsHasPlayer", IsHasPlayer);
}
