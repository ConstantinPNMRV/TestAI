// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Bot.h"
#include "Perception/AIPerceptionComponent.h"
#include "AIControllerCustom.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UCLASS()
class TESTAI_API AAIControllerCustom : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Protected|Components")
	UBlackboardComponent* BlackboardComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Protected|Bot")
	ABot* Bot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Protected|Components")
	UAIPerceptionComponent* AIPerceptionComponent;

private:
	UPROPERTY()
	UAISenseConfig_Sight* AISense_Sight;

	UPROPERTY()
	UAISenseConfig_Hearing* AISense_Hearing;

	UPROPERTY()
	UAISenseConfig_Damage* AISense_Damage;

	UPROPERTY()
	FAIStimulus StimulusCurrent;

	UPROPERTY()
	APawn* PlayerEnemy;

	UPROPERTY()
	bool IsHasPlayer = false;

public:
	explicit AAIControllerCustom(FObjectInitializer const& ObjectInitializer);

	// Задание активации дерева поведения
	UFUNCTION(BlueprintCallable, Category = "Public|Set")
	void SetEnableBehaviorTree(bool IsOn) const;

	// Задаем значение, что бот услышал звук
	UFUNCTION(BlueprintCallable, Category = "Public|Set")
	void SetIsHearNoise(bool IsOn) const;

protected:
	virtual void OnPossess(APawn* InPawn) override;

	// Подписка на делегат OnTargetPerceptionUpdated
	UFUNCTION(Category = "Protected|Bind")
	void BindOnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	// Задание точек потрулирования
	UFUNCTION(Category = "Private|Set")
	void SetPatrolLocations(FVector ForwardLocation, FVector BackwardLocation) const;

	// Вычисление точек потрулирования
	UFUNCTION(Category = "Private")
	void CalculatePatrolLocations() const;

	// Вычисление источника шума и реакция на него
	UFUNCTION(Category = "Private")
	FVector CalculateNoiseLocation(FVector StimulusLocation, FVector ReceiverLocation) const;

	// Обработка зрения
	UFUNCTION(Category = "Private")
	void BotVisionHandler();

	// Обработка слуха
	UFUNCTION(Category = "Private")
	void BotHearingHandler();

	// Обработка получения урона
	UFUNCTION(Category = "Private")
	void BotDamageHandler();
};
