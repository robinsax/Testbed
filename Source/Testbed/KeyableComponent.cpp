#include "KeyableComponent.h"

UAnchorComponent* UAnchorComponent::FindOne(AActor* Actor, FString Key) {
	TArray<UAnchorComponent*> Anchors;
	Actor->GetComponents(Anchors);

	for (int I = 0; I < Anchors.Num(); I++) {
		UAnchorComponent* Check = Anchors[I];
		if (Check->Key.Equals(Key)) return Check;
	}

	return nullptr;
}
