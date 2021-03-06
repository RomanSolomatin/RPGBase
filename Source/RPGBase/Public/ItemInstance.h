#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/NetSerialization.h"

#include "ItemInstance.generated.h"

/* An instance of an item that shows in an inventory/container, etc. */
/* Why? More lightweight than instancing the full object everywhere. */
/* Ownership: The item isn't owned, it's container is */
USTRUCT(BlueprintType)
struct RPGBASE_API FItemInstance
	: public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	FItemInstance() = default;
	FItemInstance(const FItemInstance& InSource);
	virtual ~FItemInstance() = default;

	/* Unique Id of the instance */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName Id;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FSoftClassPath ItemClass;

	/* Id of the owning container, ie. an inventory or the world. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName ContainerId;

	/* Current stack size, limited by Item's MaxStackSize */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 StackSize;

	class UItem* GetItem();

	bool IsNullItem();

	/* Returns the amount actually added to the stack (ie. if it was beyond capacity). You might customize this to add another item with the remaining amount. */
	virtual int32 AddToStack(int32 InAmount);

	/* Returns amount actually removed. */
	virtual int32 RemoveFromStack(int32 InAmount);

	virtual FItemInstance SplitStack(int32 InAmount);

	virtual FItemInstance Clone(int32 InStackSize);

	static FIntPoint GetIndex2D(int32 InColumnCount, int32 InSlot);

	void PreReplicatedRemove(const struct FItemInstanceArray& InArraySerializer) const;
	void PostReplicatedAdd(const struct FItemInstanceArray& InArraySerializer) const;

	bool operator==(const FItemInstance& InOther) const;

private:
	UPROPERTY(Transient)
	class UItem* CachedItem;
};

/* Remember to call MarkArrayDirty if an item is removed,
MarkItemDirty if added or item changed. */
USTRUCT(BlueprintType)
struct RPGBASE_API FItemInstanceArray
	: public FFastArraySerializer
{
	GENERATED_BODY()

public:
	class UContainerInstanceComponent* Owner;

	void RegisterWithOwner(class UContainerInstanceComponent* InOwner);

	/* Access this as though it were the array itself */
	operator TArray<FItemInstance>&() { return Items; }
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FItemInstance, FItemInstanceArray>(Items, DeltaParms, *this);
	}

private:
	friend struct FItemInstance;

	UPROPERTY()
	TArray<FItemInstance> Items;
};

template<>
struct TStructOpsTypeTraits<FItemInstanceArray>
	: public TStructOpsTypeTraitsBase2<FItemInstanceArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};