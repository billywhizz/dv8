// Copyright 2018 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_OBJECTS_ORDERED_HASH_TABLE_INL_H_
#define V8_OBJECTS_ORDERED_HASH_TABLE_INL_H_

#include "src/objects/ordered-hash-table.h"

#include "src/heap/heap.h"
#include "src/objects/fixed-array-inl.h"

// Has to be the last include (doesn't have include guards):
#include "src/objects/object-macros.h"

namespace v8 {
namespace internal {

CAST_ACCESSOR2(OrderedNameDictionary)
CAST_ACCESSOR2(SmallOrderedNameDictionary)
CAST_ACCESSOR2(OrderedHashMap)
CAST_ACCESSOR2(OrderedHashSet)
CAST_ACCESSOR2(SmallOrderedHashMap)
CAST_ACCESSOR2(SmallOrderedHashSet)

template <class Derived, int entrysize>
OrderedHashTable<Derived, entrysize>::OrderedHashTable(Address ptr)
    : FixedArray(ptr) {}

OrderedHashSet::OrderedHashSet(Address ptr)
    : OrderedHashTable<OrderedHashSet, 1>(ptr) {
  SLOW_DCHECK(IsOrderedHashSet());
}

OrderedHashMap::OrderedHashMap(Address ptr)
    : OrderedHashTable<OrderedHashMap, 2>(ptr) {
  SLOW_DCHECK(IsOrderedHashMap());
}

OrderedNameDictionary::OrderedNameDictionary(Address ptr)
    : OrderedHashTable<OrderedNameDictionary, 3>(ptr) {
  SLOW_DCHECK(IsOrderedNameDictionary());
}

template <class Derived>
SmallOrderedHashTable<Derived>::SmallOrderedHashTable(Address ptr)
    : HeapObjectPtr(ptr) {}

OBJECT_CONSTRUCTORS_IMPL(SmallOrderedHashSet,
                         SmallOrderedHashTable<SmallOrderedHashSet>)
OBJECT_CONSTRUCTORS_IMPL(SmallOrderedHashMap,
                         SmallOrderedHashTable<SmallOrderedHashMap>)
OBJECT_CONSTRUCTORS_IMPL(SmallOrderedNameDictionary,
                         SmallOrderedHashTable<SmallOrderedNameDictionary>)

RootIndex OrderedHashSet::GetMapRootIndex() {
  return RootIndex::kOrderedHashSetMap;
}

RootIndex OrderedHashMap::GetMapRootIndex() {
  return RootIndex::kOrderedHashMapMap;
}

RootIndex OrderedNameDictionary::GetMapRootIndex() {
  return RootIndex::kOrderedNameDictionaryMap;
}

RootIndex SmallOrderedNameDictionary::GetMapRootIndex() {
  return RootIndex::kSmallOrderedNameDictionaryMap;
}

RootIndex SmallOrderedHashMap::GetMapRootIndex() {
  return RootIndex::kSmallOrderedHashMapMap;
}

RootIndex SmallOrderedHashSet::GetMapRootIndex() {
  return RootIndex::kSmallOrderedHashSetMap;
}

inline Object* OrderedHashMap::ValueAt(int entry) {
  DCHECK_NE(entry, kNotFound);
  DCHECK_LT(entry, UsedCapacity());
  return get(EntryToIndex(entry) + kValueOffset);
}

inline Object* OrderedNameDictionary::ValueAt(int entry) {
  DCHECK_NE(entry, kNotFound);
  DCHECK_LT(entry, UsedCapacity());
  return get(EntryToIndex(entry) + kValueOffset);
}

// Set the value for entry.
inline void OrderedNameDictionary::ValueAtPut(int entry, Object* value) {
  DCHECK_NE(entry, kNotFound);
  DCHECK_LT(entry, UsedCapacity());
  this->set(EntryToIndex(entry) + kValueOffset, value);
}

// Returns the property details for the property at entry.
inline PropertyDetails OrderedNameDictionary::DetailsAt(int entry) {
  DCHECK_NE(entry, kNotFound);
  DCHECK_LT(entry, this->UsedCapacity());
  // TODO(gsathya): Optimize the cast away.
  return PropertyDetails(
      Smi::cast(get(EntryToIndex(entry) + kPropertyDetailsOffset)));
}

inline void OrderedNameDictionary::DetailsAtPut(int entry,
                                                PropertyDetails value) {
  DCHECK_NE(entry, kNotFound);
  DCHECK_LT(entry, this->UsedCapacity());
  // TODO(gsathya): Optimize the cast away.
  this->set(EntryToIndex(entry) + kPropertyDetailsOffset, value.AsSmi());
}

inline Object* SmallOrderedNameDictionary::ValueAt(int entry) {
  return this->GetDataEntry(entry, kValueIndex);
}

// Set the value for entry.
inline void SmallOrderedNameDictionary::ValueAtPut(int entry, Object* value) {
  this->SetDataEntry(entry, kValueIndex, value);
}

// Returns the property details for the property at entry.
inline PropertyDetails SmallOrderedNameDictionary::DetailsAt(int entry) {
  // TODO(gsathya): Optimize the cast away. And store this in the data table.
  return PropertyDetails(
      Smi::cast(this->GetDataEntry(entry, kPropertyDetailsIndex)));
}

// Set the details for entry.
inline void SmallOrderedNameDictionary::DetailsAtPut(int entry,
                                                     PropertyDetails value) {
  // TODO(gsathya): Optimize the cast away. And store this in the data table.
  this->SetDataEntry(entry, kPropertyDetailsIndex, value.AsSmi());
}

inline bool OrderedHashSet::Is(Handle<HeapObject> table) {
  return table->IsOrderedHashSet();
}

inline bool OrderedHashMap::Is(Handle<HeapObject> table) {
  return table->IsOrderedHashMap();
}

inline bool SmallOrderedHashSet::Is(Handle<HeapObject> table) {
  return table->IsSmallOrderedHashSet();
}

inline bool SmallOrderedHashMap::Is(Handle<HeapObject> table) {
  return table->IsSmallOrderedHashMap();
}

template <class Derived>
void SmallOrderedHashTable<Derived>::SetDataEntry(int entry, int relative_index,
                                                  Object* value) {
  DCHECK_NE(kNotFound, entry);
  Address entry_offset = GetDataEntryOffset(entry, relative_index);
  RELAXED_WRITE_FIELD(this, entry_offset, value);
  WRITE_BARRIER(this, static_cast<int>(entry_offset), value);
}

template <class Derived, class TableType>
Object* OrderedHashTableIterator<Derived, TableType>::CurrentKey() {
  TableType table = TableType::cast(this->table());
  int index = Smi::ToInt(this->index());
  Object* key = table->KeyAt(index);
  DCHECK(!key->IsTheHole());
  return key;
}

}  // namespace internal
}  // namespace v8

#include "src/objects/object-macros-undef.h"

#endif  // V8_OBJECTS_ORDERED_HASH_TABLE_INL_H_
