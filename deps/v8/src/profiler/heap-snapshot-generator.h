// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_PROFILER_HEAP_SNAPSHOT_GENERATOR_H_
#define V8_PROFILER_HEAP_SNAPSHOT_GENERATOR_H_

#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "include/v8-profiler.h"
#include "src/base/platform/time.h"
#include "src/objects.h"
#include "src/objects/fixed-array.h"
#include "src/objects/hash-table.h"
#include "src/objects/literal-objects.h"
#include "src/profiler/strings-storage.h"
#include "src/string-hasher.h"
#include "src/visitors.h"

namespace v8 {
namespace internal {

class AllocationTracker;
class AllocationTraceNode;
class HeapEntry;
class HeapIterator;
class HeapProfiler;
class HeapSnapshot;
class JSArrayBuffer;
class JSCollection;
class JSGeneratorObject;
class JSGlobalObject;
class JSGlobalProxy;
class JSPromise;
class JSWeakCollection;
class SnapshotFiller;

struct SourceLocation {
  SourceLocation(int entry_index, int scriptId, int line, int col)
      : entry_index(entry_index), scriptId(scriptId), line(line), col(col) {}

  const int entry_index;
  const int scriptId;
  const int line;
  const int col;
};

class HeapGraphEdge {
 public:
  enum Type {
    kContextVariable = v8::HeapGraphEdge::kContextVariable,
    kElement = v8::HeapGraphEdge::kElement,
    kProperty = v8::HeapGraphEdge::kProperty,
    kInternal = v8::HeapGraphEdge::kInternal,
    kHidden = v8::HeapGraphEdge::kHidden,
    kShortcut = v8::HeapGraphEdge::kShortcut,
    kWeak = v8::HeapGraphEdge::kWeak
  };

  HeapGraphEdge(Type type, const char* name, int from, int to);
  HeapGraphEdge(Type type, int index, int from, int to);
  void ReplaceToIndexWithEntry(HeapSnapshot* snapshot);

  Type type() const { return TypeField::decode(bit_field_); }
  int index() const {
    DCHECK(type() == kElement || type() == kHidden);
    return index_;
  }
  const char* name() const {
    DCHECK(type() == kContextVariable || type() == kProperty ||
           type() == kInternal || type() == kShortcut || type() == kWeak);
    return name_;
  }
  V8_INLINE HeapEntry* from() const;
  HeapEntry* to() const { return to_entry_; }

  V8_INLINE Isolate* isolate() const;

 private:
  V8_INLINE HeapSnapshot* snapshot() const;
  int from_index() const { return FromIndexField::decode(bit_field_); }

  class TypeField : public BitField<Type, 0, 3> {};
  class FromIndexField : public BitField<int, 3, 29> {};
  uint32_t bit_field_;
  union {
    // During entries population |to_index_| is used for storing the index,
    // afterwards it is replaced with a pointer to the entry.
    int to_index_;
    HeapEntry* to_entry_;
  };
  union {
    int index_;
    const char* name_;
  };
};


// HeapEntry instances represent an entity from the heap (or a special
// virtual node, e.g. root).
class HeapEntry {
 public:
  enum Type {
    kHidden = v8::HeapGraphNode::kHidden,
    kArray = v8::HeapGraphNode::kArray,
    kString = v8::HeapGraphNode::kString,
    kObject = v8::HeapGraphNode::kObject,
    kCode = v8::HeapGraphNode::kCode,
    kClosure = v8::HeapGraphNode::kClosure,
    kRegExp = v8::HeapGraphNode::kRegExp,
    kHeapNumber = v8::HeapGraphNode::kHeapNumber,
    kNative = v8::HeapGraphNode::kNative,
    kSynthetic = v8::HeapGraphNode::kSynthetic,
    kConsString = v8::HeapGraphNode::kConsString,
    kSlicedString = v8::HeapGraphNode::kSlicedString,
    kSymbol = v8::HeapGraphNode::kSymbol,
    kBigInt = v8::HeapGraphNode::kBigInt
  };
  static const int kNoEntry;

  HeapEntry() = default;
  HeapEntry(HeapSnapshot* snapshot,
            Type type,
            const char* name,
            SnapshotObjectId id,
            size_t self_size,
            unsigned trace_node_id);

  HeapSnapshot* snapshot() { return snapshot_; }
  Type type() const { return static_cast<Type>(type_); }
  void set_type(Type type) { type_ = type; }
  const char* name() const { return name_; }
  void set_name(const char* name) { name_ = name; }
  SnapshotObjectId id() const { return id_; }
  size_t self_size() const { return self_size_; }
  unsigned trace_node_id() const { return trace_node_id_; }
  V8_INLINE int index() const;
  int children_count() const { return children_count_; }
  V8_INLINE int set_children_index(int index);
  V8_INLINE void add_child(HeapGraphEdge* edge);
  V8_INLINE HeapGraphEdge* child(int i);
  V8_INLINE Isolate* isolate() const;

  void SetIndexedReference(
      HeapGraphEdge::Type type, int index, HeapEntry* entry);
  void SetNamedReference(
      HeapGraphEdge::Type type, const char* name, HeapEntry* entry);

  void Print(
      const char* prefix, const char* edge_name, int max_depth, int indent);

 private:
  V8_INLINE std::deque<HeapGraphEdge*>::iterator children_begin();
  V8_INLINE std::deque<HeapGraphEdge*>::iterator children_end();
  const char* TypeAsString();

  unsigned type_: 4;
  int children_count_: 28;
  int children_index_;
  size_t self_size_;
  HeapSnapshot* snapshot_;
  const char* name_;
  SnapshotObjectId id_;
  // id of allocation stack trace top node
  unsigned trace_node_id_;
};


// HeapSnapshot represents a single heap snapshot. It is stored in
// HeapProfiler, which is also a factory for
// HeapSnapshots. All HeapSnapshots share strings copied from JS heap
// to be able to return them even if they were collected.
// HeapSnapshotGenerator fills in a HeapSnapshot.
class HeapSnapshot {
 public:
  explicit HeapSnapshot(HeapProfiler* profiler);
  void Delete();

  HeapProfiler* profiler() { return profiler_; }
  HeapEntry* root() { return &entries_[root_index_]; }
  HeapEntry* gc_roots() { return &entries_[gc_roots_index_]; }
  HeapEntry* gc_subroot(Root root) {
    return &entries_[gc_subroot_indexes_[static_cast<int>(root)]];
  }
  std::vector<HeapEntry>& entries() { return entries_; }
  std::deque<HeapGraphEdge>& edges() { return edges_; }
  std::deque<HeapGraphEdge*>& children() { return children_; }
  const std::vector<SourceLocation>& locations() const { return locations_; }
  void RememberLastJSObjectId();
  SnapshotObjectId max_snapshot_js_object_id() const {
    return max_snapshot_js_object_id_;
  }

  void AddLocation(int entry, int scriptId, int line, int col);
  HeapEntry* AddEntry(HeapEntry::Type type,
                      const char* name,
                      SnapshotObjectId id,
                      size_t size,
                      unsigned trace_node_id);
  void AddSyntheticRootEntries();
  HeapEntry* GetEntryById(SnapshotObjectId id);
  std::vector<HeapEntry*>* GetSortedEntriesList();
  void FillChildren();

  void Print(int max_depth);

 private:
  HeapEntry* AddRootEntry();
  HeapEntry* AddGcRootsEntry();
  HeapEntry* AddGcSubrootEntry(Root root, SnapshotObjectId id);

  HeapProfiler* profiler_;
  int root_index_;
  int gc_roots_index_;
  int gc_subroot_indexes_[static_cast<int>(Root::kNumberOfRoots)];
  std::vector<HeapEntry> entries_;
  std::deque<HeapGraphEdge> edges_;
  std::deque<HeapGraphEdge*> children_;
  std::vector<HeapEntry*> sorted_entries_;
  std::vector<SourceLocation> locations_;
  SnapshotObjectId max_snapshot_js_object_id_;

  friend class HeapSnapshotTester;

  DISALLOW_COPY_AND_ASSIGN(HeapSnapshot);
};


class HeapObjectsMap {
 public:
  struct TimeInterval {
    explicit TimeInterval(SnapshotObjectId id)
        : id(id), size(0), count(0), timestamp(base::TimeTicks::Now()) {}
    SnapshotObjectId last_assigned_id() const { return id - kObjectIdStep; }
    SnapshotObjectId id;
    uint32_t size;
    uint32_t count;
    base::TimeTicks timestamp;
  };

  explicit HeapObjectsMap(Heap* heap);

  Heap* heap() const { return heap_; }

  SnapshotObjectId FindEntry(Address addr);
  SnapshotObjectId FindOrAddEntry(Address addr,
                                  unsigned int size,
                                  bool accessed = true);
  bool MoveObject(Address from, Address to, int size);
  void UpdateObjectSize(Address addr, int size);
  SnapshotObjectId last_assigned_id() const {
    return next_id_ - kObjectIdStep;
  }

  void StopHeapObjectsTracking();
  SnapshotObjectId PushHeapObjectsStats(OutputStream* stream,
                                        int64_t* timestamp_us);
  const std::vector<TimeInterval>& samples() const { return time_intervals_; }

  SnapshotObjectId GenerateId(v8::RetainedObjectInfo* info);

  static const int kObjectIdStep = 2;
  static const SnapshotObjectId kInternalRootObjectId;
  static const SnapshotObjectId kGcRootsObjectId;
  static const SnapshotObjectId kGcRootsFirstSubrootId;
  static const SnapshotObjectId kFirstAvailableObjectId;

  void UpdateHeapObjectsMap();
  void RemoveDeadEntries();

 private:
  struct EntryInfo {
    EntryInfo(SnapshotObjectId id, Address addr, unsigned int size,
              bool accessed)
        : id(id), addr(addr), size(size), accessed(accessed) {}
    SnapshotObjectId id;
    Address addr;
    unsigned int size;
    bool accessed;
  };

  SnapshotObjectId next_id_;
  // TODO(jkummerow): Use a map that uses {Address} as the key type.
  base::HashMap entries_map_;
  std::vector<EntryInfo> entries_;
  std::vector<TimeInterval> time_intervals_;
  Heap* heap_;

  DISALLOW_COPY_AND_ASSIGN(HeapObjectsMap);
};

// A typedef for referencing anything that can be snapshotted living
// in any kind of heap memory.
typedef void* HeapThing;

// An interface that creates HeapEntries by HeapThings.
class HeapEntriesAllocator {
 public:
  virtual ~HeapEntriesAllocator() = default;
  virtual HeapEntry* AllocateEntry(HeapThing ptr) = 0;
};

class SnapshottingProgressReportingInterface {
 public:
  virtual ~SnapshottingProgressReportingInterface() = default;
  virtual void ProgressStep() = 0;
  virtual bool ProgressReport(bool force) = 0;
};

// An implementation of V8 heap graph extractor.
class V8HeapExplorer : public HeapEntriesAllocator {
 public:
  V8HeapExplorer(HeapSnapshot* snapshot,
                 SnapshottingProgressReportingInterface* progress,
                 v8::HeapProfiler::ObjectNameResolver* resolver);
  ~V8HeapExplorer() override = default;
  HeapEntry* AllocateEntry(HeapThing ptr) override;
  int EstimateObjectsCount();
  bool IterateAndExtractReferences(SnapshotFiller* filler);
  void TagGlobalObjects();
  void TagCodeObject(Code* code);
  void TagBuiltinCodeObject(Code* code, const char* name);
  HeapEntry* AddEntry(Address address,
                      HeapEntry::Type type,
                      const char* name,
                      size_t size);

  static JSFunction* GetConstructor(JSReceiver* receiver);
  static String* GetConstructorName(JSObject* object);

 private:
  void MarkVisitedField(int offset);

  HeapEntry* AddEntry(HeapObject* object);
  HeapEntry* AddEntry(HeapObject* object,
                      HeapEntry::Type type,
                      const char* name);

  const char* GetSystemEntryName(HeapObject* object);

  void ExtractLocation(int entry, HeapObject* object);
  void ExtractLocationForJSFunction(int entry, JSFunction* func);
  void ExtractReferences(int entry, HeapObject* obj);
  void ExtractJSGlobalProxyReferences(int entry, JSGlobalProxy* proxy);
  void ExtractJSObjectReferences(int entry, JSObject* js_obj);
  void ExtractStringReferences(int entry, String* obj);
  void ExtractSymbolReferences(int entry, Symbol* symbol);
  void ExtractJSCollectionReferences(int entry, JSCollection* collection);
  void ExtractJSWeakCollectionReferences(int entry,
                                         JSWeakCollection* collection);
  void ExtractEphemeronHashTableReferences(int entry,
                                           EphemeronHashTable* table);
  void ExtractContextReferences(int entry, Context* context);
  void ExtractMapReferences(int entry, Map* map);
  void ExtractSharedFunctionInfoReferences(int entry,
                                           SharedFunctionInfo* shared);
  void ExtractScriptReferences(int entry, Script* script);
  void ExtractAccessorInfoReferences(int entry, AccessorInfo* accessor_info);
  void ExtractAccessorPairReferences(int entry, AccessorPair* accessors);
  void ExtractCodeReferences(int entry, Code* code);
  void ExtractCellReferences(int entry, Cell* cell);
  void ExtractFeedbackCellReferences(int entry, FeedbackCell* feedback_cell);
  void ExtractPropertyCellReferences(int entry, PropertyCell* cell);
  void ExtractAllocationSiteReferences(int entry, AllocationSite* site);
  void ExtractArrayBoilerplateDescriptionReferences(
      int entry, ArrayBoilerplateDescription* value);
  void ExtractJSArrayBufferReferences(int entry, JSArrayBuffer* buffer);
  void ExtractJSPromiseReferences(int entry, JSPromise* promise);
  void ExtractJSGeneratorObjectReferences(int entry,
                                          JSGeneratorObject* generator);
  void ExtractFixedArrayReferences(int entry, FixedArray* array);
  void ExtractFeedbackVectorReferences(int entry,
                                       FeedbackVector* feedback_vector);
  template <typename T>
  void ExtractWeakArrayReferences(int header_size, int entry, T* array);
  void ExtractPropertyReferences(JSObject* js_obj, int entry);
  void ExtractAccessorPairProperty(JSObject* js_obj, int entry, Name* key,
                                   Object* callback_obj, int field_offset = -1);
  void ExtractElementReferences(JSObject* js_obj, int entry);
  void ExtractInternalReferences(JSObject* js_obj, int entry);

  bool IsEssentialObject(Object* object);
  bool IsEssentialHiddenReference(Object* parent, int field_offset);

  void SetContextReference(HeapObject* parent_obj,
                           int parent,
                           String* reference_name,
                           Object* child,
                           int field_offset);
  void SetNativeBindReference(HeapObject* parent_obj,
                              int parent,
                              const char* reference_name,
                              Object* child);
  void SetElementReference(HeapObject* parent_obj,
                           int parent,
                           int index,
                           Object* child);
  void SetInternalReference(HeapObject* parent_obj,
                            int parent,
                            const char* reference_name,
                            Object* child,
                            int field_offset = -1);
  void SetInternalReference(HeapObject* parent_obj,
                            int parent,
                            int index,
                            Object* child,
                            int field_offset = -1);
  void SetHiddenReference(HeapObject* parent_obj, int parent, int index,
                          Object* child, int field_offset);
  void SetWeakReference(HeapObject* parent_obj,
                        int parent,
                        const char* reference_name,
                        Object* child_obj,
                        int field_offset);
  void SetWeakReference(HeapObject* parent_obj,
                        int parent,
                        int index,
                        Object* child_obj,
                        int field_offset);
  void SetPropertyReference(HeapObject* parent_obj, int parent,
                            Name* reference_name, Object* child,
                            const char* name_format_string = nullptr,
                            int field_offset = -1);
  void SetDataOrAccessorPropertyReference(
      PropertyKind kind, JSObject* parent_obj, int parent, Name* reference_name,
      Object* child, const char* name_format_string = nullptr,
      int field_offset = -1);

  void SetUserGlobalReference(Object* user_global);
  void SetRootGcRootsReference();
  void SetGcRootsReference(Root root);
  void SetGcSubrootReference(Root root, const char* description, bool is_weak,
                             Object* child);
  const char* GetStrongGcSubrootName(Object* object);
  void TagObject(Object* obj, const char* tag);

  HeapEntry* GetEntry(Object* obj);

  Heap* heap_;
  HeapSnapshot* snapshot_;
  StringsStorage* names_;
  HeapObjectsMap* heap_object_map_;
  SnapshottingProgressReportingInterface* progress_;
  SnapshotFiller* filler_;
  std::unordered_map<JSGlobalObject*, const char*> objects_tags_;
  std::unordered_map<Object*, const char*> strong_gc_subroot_names_;
  std::unordered_set<JSGlobalObject*> user_roots_;
  v8::HeapProfiler::ObjectNameResolver* global_object_name_resolver_;

  std::vector<bool> visited_fields_;

  friend class IndexedReferencesExtractor;
  friend class RootsReferencesExtractor;

  DISALLOW_COPY_AND_ASSIGN(V8HeapExplorer);
};


class NativeGroupRetainedObjectInfo;


// An implementation of retained native objects extractor.
class NativeObjectsExplorer {
 public:
  NativeObjectsExplorer(HeapSnapshot* snapshot,
                        SnapshottingProgressReportingInterface* progress);
  virtual ~NativeObjectsExplorer();
  int EstimateObjectsCount();
  bool IterateAndExtractReferences(SnapshotFiller* filler);

 private:
  void FillRetainedObjects();
  void FillEdges();
  std::vector<HeapObject*>* GetVectorMaybeDisposeInfo(
      v8::RetainedObjectInfo* info);
  void SetNativeRootReference(v8::RetainedObjectInfo* info);
  void SetRootNativeRootsReference();
  void SetWrapperNativeReferences(HeapObject* wrapper,
                                      v8::RetainedObjectInfo* info);
  void VisitSubtreeWrapper(Object** p, uint16_t class_id);

  struct RetainedInfoHasher {
    std::size_t operator()(v8::RetainedObjectInfo* info) const {
      return ComputeUnseededHash(static_cast<uint32_t>(info->GetHash()));
    }
  };
  struct RetainedInfoEquals {
    bool operator()(v8::RetainedObjectInfo* info1,
                    v8::RetainedObjectInfo* info2) const {
      return info1 == info2 || info1->IsEquivalent(info2);
    }
  };

  NativeGroupRetainedObjectInfo* FindOrAddGroupInfo(const char* label);

  HeapEntry* EntryForEmbedderGraphNode(EmbedderGraph::Node* node);

  Isolate* isolate_;
  HeapSnapshot* snapshot_;
  StringsStorage* names_;
  bool embedder_queried_;
  std::unordered_set<Object*> in_groups_;
  std::unordered_map<v8::RetainedObjectInfo*, std::vector<HeapObject*>*,
                     RetainedInfoHasher, RetainedInfoEquals>
      objects_by_info_;
  std::unordered_map<const char*, NativeGroupRetainedObjectInfo*,
                     SeededStringHasher, StringEquals>
      native_groups_;
  std::unique_ptr<HeapEntriesAllocator> synthetic_entries_allocator_;
  std::unique_ptr<HeapEntriesAllocator> native_entries_allocator_;
  std::unique_ptr<HeapEntriesAllocator> embedder_graph_entries_allocator_;
  // Used during references extraction.
  SnapshotFiller* filler_;
  v8::HeapProfiler::RetainerEdges edges_;

  static HeapThing const kNativesRootObject;

  friend class GlobalHandlesExtractor;

  DISALLOW_COPY_AND_ASSIGN(NativeObjectsExplorer);
};


class HeapSnapshotGenerator : public SnapshottingProgressReportingInterface {
 public:
  // The HeapEntriesMap instance is used to track a mapping between
  // real heap objects and their representations in heap snapshots.
  using HeapEntriesMap = std::unordered_map<HeapThing, int>;

  HeapSnapshotGenerator(HeapSnapshot* snapshot,
                        v8::ActivityControl* control,
                        v8::HeapProfiler::ObjectNameResolver* resolver,
                        Heap* heap);
  bool GenerateSnapshot();

 private:
  bool FillReferences();
  void ProgressStep() override;
  bool ProgressReport(bool force = false) override;
  void InitProgressCounter();

  HeapSnapshot* snapshot_;
  v8::ActivityControl* control_;
  V8HeapExplorer v8_heap_explorer_;
  NativeObjectsExplorer dom_explorer_;
  // Mapping from HeapThing pointers to HeapEntry indices.
  HeapEntriesMap entries_map_;
  // Used during snapshot generation.
  int progress_counter_;
  int progress_total_;
  Heap* heap_;

  DISALLOW_COPY_AND_ASSIGN(HeapSnapshotGenerator);
};

class OutputStreamWriter;

class HeapSnapshotJSONSerializer {
 public:
  explicit HeapSnapshotJSONSerializer(HeapSnapshot* snapshot)
      : snapshot_(snapshot),
        strings_(StringsMatch),
        next_node_id_(1),
        next_string_id_(1),
        writer_(nullptr) {}
  void Serialize(v8::OutputStream* stream);

 private:
  V8_INLINE static bool StringsMatch(void* key1, void* key2) {
    return strcmp(reinterpret_cast<char*>(key1),
                  reinterpret_cast<char*>(key2)) == 0;
  }

  V8_INLINE static uint32_t StringHash(const void* string);

  int GetStringId(const char* s);
  V8_INLINE int to_node_index(const HeapEntry* e);
  V8_INLINE int to_node_index(int entry_index);
  void SerializeEdge(HeapGraphEdge* edge, bool first_edge);
  void SerializeEdges();
  void SerializeImpl();
  void SerializeNode(const HeapEntry* entry);
  void SerializeNodes();
  void SerializeSnapshot();
  void SerializeTraceTree();
  void SerializeTraceNode(AllocationTraceNode* node);
  void SerializeTraceNodeInfos();
  void SerializeSamples();
  void SerializeString(const unsigned char* s);
  void SerializeStrings();
  void SerializeLocation(const SourceLocation& location);
  void SerializeLocations();

  static const int kEdgeFieldsCount;
  static const int kNodeFieldsCount;

  HeapSnapshot* snapshot_;
  base::CustomMatcherHashMap strings_;
  int next_node_id_;
  int next_string_id_;
  OutputStreamWriter* writer_;

  friend class HeapSnapshotJSONSerializerEnumerator;
  friend class HeapSnapshotJSONSerializerIterator;

  DISALLOW_COPY_AND_ASSIGN(HeapSnapshotJSONSerializer);
};


}  // namespace internal
}  // namespace v8

#endif  // V8_PROFILER_HEAP_SNAPSHOT_GENERATOR_H_