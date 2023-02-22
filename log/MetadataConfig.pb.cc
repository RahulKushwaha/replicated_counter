// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: MetadataConfig.proto

#include "MetadataConfig.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG

namespace _pb = ::PROTOBUF_NAMESPACE_ID;
namespace _pbi = _pb::internal;

namespace rk {
namespace projects {
namespace durable_log {
PROTOBUF_CONSTEXPR SequencerConfig::SequencerConfig(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.id_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct SequencerConfigDefaultTypeInternal {
  PROTOBUF_CONSTEXPR SequencerConfigDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~SequencerConfigDefaultTypeInternal() {}
  union {
    SequencerConfig _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 SequencerConfigDefaultTypeInternal _SequencerConfig_default_instance_;
PROTOBUF_CONSTEXPR ReplicaConfig::ReplicaConfig(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.id_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct ReplicaConfigDefaultTypeInternal {
  PROTOBUF_CONSTEXPR ReplicaConfigDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~ReplicaConfigDefaultTypeInternal() {}
  union {
    ReplicaConfig _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 ReplicaConfigDefaultTypeInternal _ReplicaConfig_default_instance_;
PROTOBUF_CONSTEXPR MetadataConfig::MetadataConfig(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.replica_set_config_)*/{}
  , /*decltype(_impl_.sequencer_config_)*/nullptr
  , /*decltype(_impl_.version_id_)*/int64_t{0}
  , /*decltype(_impl_.previous_version_id_)*/int64_t{0}
  , /*decltype(_impl_.start_index_)*/int64_t{0}
  , /*decltype(_impl_.end_index_)*/int64_t{0}
  , /*decltype(_impl_.previous_version_end_index_)*/int64_t{0}
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct MetadataConfigDefaultTypeInternal {
  PROTOBUF_CONSTEXPR MetadataConfigDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~MetadataConfigDefaultTypeInternal() {}
  union {
    MetadataConfig _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 MetadataConfigDefaultTypeInternal _MetadataConfig_default_instance_;
}  // namespace durable_log
}  // namespace projects
}  // namespace rk
static ::_pb::Metadata file_level_metadata_MetadataConfig_2eproto[3];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_MetadataConfig_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_MetadataConfig_2eproto = nullptr;

const uint32_t TableStruct_MetadataConfig_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::SequencerConfig, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::SequencerConfig, _impl_.id_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::ReplicaConfig, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::ReplicaConfig, _impl_.id_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::MetadataConfig, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::MetadataConfig, _impl_.version_id_),
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::MetadataConfig, _impl_.previous_version_id_),
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::MetadataConfig, _impl_.start_index_),
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::MetadataConfig, _impl_.end_index_),
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::MetadataConfig, _impl_.previous_version_end_index_),
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::MetadataConfig, _impl_.sequencer_config_),
  PROTOBUF_FIELD_OFFSET(::rk::projects::durable_log::MetadataConfig, _impl_.replica_set_config_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::rk::projects::durable_log::SequencerConfig)},
  { 7, -1, -1, sizeof(::rk::projects::durable_log::ReplicaConfig)},
  { 14, -1, -1, sizeof(::rk::projects::durable_log::MetadataConfig)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::rk::projects::durable_log::_SequencerConfig_default_instance_._instance,
  &::rk::projects::durable_log::_ReplicaConfig_default_instance_._instance,
  &::rk::projects::durable_log::_MetadataConfig_default_instance_._instance,
};

const char descriptor_table_protodef_MetadataConfig_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\024MetadataConfig.proto\022\027rk.projects.dura"
  "ble_log\"\035\n\017SequencerConfig\022\n\n\002id\030\001 \001(\t\"\033"
  "\n\rReplicaConfig\022\n\n\002id\030\001 \001(\t\"\225\002\n\016Metadata"
  "Config\022\022\n\nversion_id\030\001 \001(\003\022\033\n\023previous_v"
  "ersion_id\030\002 \001(\003\022\023\n\013start_index\030\003 \001(\003\022\021\n\t"
  "end_index\030\004 \001(\003\022\"\n\032previous_version_end_"
  "index\030\005 \001(\003\022B\n\020sequencer_config\030\006 \001(\0132(."
  "rk.projects.durable_log.SequencerConfig\022"
  "B\n\022replica_set_config\030\007 \003(\0132&.rk.project"
  "s.durable_log.ReplicaConfigb\006proto3"
  ;
static ::_pbi::once_flag descriptor_table_MetadataConfig_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_MetadataConfig_2eproto = {
    false, false, 395, descriptor_table_protodef_MetadataConfig_2eproto,
    "MetadataConfig.proto",
    &descriptor_table_MetadataConfig_2eproto_once, nullptr, 0, 3,
    schemas, file_default_instances, TableStruct_MetadataConfig_2eproto::offsets,
    file_level_metadata_MetadataConfig_2eproto, file_level_enum_descriptors_MetadataConfig_2eproto,
    file_level_service_descriptors_MetadataConfig_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_MetadataConfig_2eproto_getter() {
  return &descriptor_table_MetadataConfig_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_MetadataConfig_2eproto(&descriptor_table_MetadataConfig_2eproto);
namespace rk {
namespace projects {
namespace durable_log {

// ===================================================================

class SequencerConfig::_Internal {
 public:
};

SequencerConfig::SequencerConfig(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:rk.projects.durable_log.SequencerConfig)
}
SequencerConfig::SequencerConfig(const SequencerConfig& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  SequencerConfig* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.id_){}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _impl_.id_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.id_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_id().empty()) {
    _this->_impl_.id_.Set(from._internal_id(), 
      _this->GetArenaForAllocation());
  }
  // @@protoc_insertion_point(copy_constructor:rk.projects.durable_log.SequencerConfig)
}

inline void SequencerConfig::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.id_){}
    , /*decltype(_impl_._cached_size_)*/{}
  };
  _impl_.id_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.id_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
}

SequencerConfig::~SequencerConfig() {
  // @@protoc_insertion_point(destructor:rk.projects.durable_log.SequencerConfig)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void SequencerConfig::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.id_.Destroy();
}

void SequencerConfig::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void SequencerConfig::Clear() {
// @@protoc_insertion_point(message_clear_start:rk.projects.durable_log.SequencerConfig)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.id_.ClearToEmpty();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* SequencerConfig::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string id = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          auto str = _internal_mutable_id();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "rk.projects.durable_log.SequencerConfig.id"));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* SequencerConfig::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:rk.projects.durable_log.SequencerConfig)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // string id = 1;
  if (!this->_internal_id().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_id().data(), static_cast<int>(this->_internal_id().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "rk.projects.durable_log.SequencerConfig.id");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_id(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:rk.projects.durable_log.SequencerConfig)
  return target;
}

size_t SequencerConfig::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:rk.projects.durable_log.SequencerConfig)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string id = 1;
  if (!this->_internal_id().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_id());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData SequencerConfig::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    SequencerConfig::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*SequencerConfig::GetClassData() const { return &_class_data_; }


void SequencerConfig::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<SequencerConfig*>(&to_msg);
  auto& from = static_cast<const SequencerConfig&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:rk.projects.durable_log.SequencerConfig)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_id().empty()) {
    _this->_internal_set_id(from._internal_id());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void SequencerConfig::CopyFrom(const SequencerConfig& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:rk.projects.durable_log.SequencerConfig)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool SequencerConfig::IsInitialized() const {
  return true;
}

void SequencerConfig::InternalSwap(SequencerConfig* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.id_, lhs_arena,
      &other->_impl_.id_, rhs_arena
  );
}

::PROTOBUF_NAMESPACE_ID::Metadata SequencerConfig::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_MetadataConfig_2eproto_getter, &descriptor_table_MetadataConfig_2eproto_once,
      file_level_metadata_MetadataConfig_2eproto[0]);
}

// ===================================================================

class ReplicaConfig::_Internal {
 public:
};

ReplicaConfig::ReplicaConfig(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:rk.projects.durable_log.ReplicaConfig)
}
ReplicaConfig::ReplicaConfig(const ReplicaConfig& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  ReplicaConfig* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.id_){}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _impl_.id_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.id_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_id().empty()) {
    _this->_impl_.id_.Set(from._internal_id(), 
      _this->GetArenaForAllocation());
  }
  // @@protoc_insertion_point(copy_constructor:rk.projects.durable_log.ReplicaConfig)
}

inline void ReplicaConfig::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.id_){}
    , /*decltype(_impl_._cached_size_)*/{}
  };
  _impl_.id_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.id_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
}

ReplicaConfig::~ReplicaConfig() {
  // @@protoc_insertion_point(destructor:rk.projects.durable_log.ReplicaConfig)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void ReplicaConfig::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.id_.Destroy();
}

void ReplicaConfig::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void ReplicaConfig::Clear() {
// @@protoc_insertion_point(message_clear_start:rk.projects.durable_log.ReplicaConfig)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.id_.ClearToEmpty();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* ReplicaConfig::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // string id = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          auto str = _internal_mutable_id();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "rk.projects.durable_log.ReplicaConfig.id"));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* ReplicaConfig::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:rk.projects.durable_log.ReplicaConfig)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // string id = 1;
  if (!this->_internal_id().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_id().data(), static_cast<int>(this->_internal_id().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "rk.projects.durable_log.ReplicaConfig.id");
    target = stream->WriteStringMaybeAliased(
        1, this->_internal_id(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:rk.projects.durable_log.ReplicaConfig)
  return target;
}

size_t ReplicaConfig::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:rk.projects.durable_log.ReplicaConfig)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string id = 1;
  if (!this->_internal_id().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_id());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData ReplicaConfig::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    ReplicaConfig::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*ReplicaConfig::GetClassData() const { return &_class_data_; }


void ReplicaConfig::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<ReplicaConfig*>(&to_msg);
  auto& from = static_cast<const ReplicaConfig&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:rk.projects.durable_log.ReplicaConfig)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_id().empty()) {
    _this->_internal_set_id(from._internal_id());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void ReplicaConfig::CopyFrom(const ReplicaConfig& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:rk.projects.durable_log.ReplicaConfig)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool ReplicaConfig::IsInitialized() const {
  return true;
}

void ReplicaConfig::InternalSwap(ReplicaConfig* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.id_, lhs_arena,
      &other->_impl_.id_, rhs_arena
  );
}

::PROTOBUF_NAMESPACE_ID::Metadata ReplicaConfig::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_MetadataConfig_2eproto_getter, &descriptor_table_MetadataConfig_2eproto_once,
      file_level_metadata_MetadataConfig_2eproto[1]);
}

// ===================================================================

class MetadataConfig::_Internal {
 public:
  static const ::rk::projects::durable_log::SequencerConfig& sequencer_config(const MetadataConfig* msg);
};

const ::rk::projects::durable_log::SequencerConfig&
MetadataConfig::_Internal::sequencer_config(const MetadataConfig* msg) {
  return *msg->_impl_.sequencer_config_;
}
MetadataConfig::MetadataConfig(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:rk.projects.durable_log.MetadataConfig)
}
MetadataConfig::MetadataConfig(const MetadataConfig& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  MetadataConfig* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.replica_set_config_){from._impl_.replica_set_config_}
    , decltype(_impl_.sequencer_config_){nullptr}
    , decltype(_impl_.version_id_){}
    , decltype(_impl_.previous_version_id_){}
    , decltype(_impl_.start_index_){}
    , decltype(_impl_.end_index_){}
    , decltype(_impl_.previous_version_end_index_){}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  if (from._internal_has_sequencer_config()) {
    _this->_impl_.sequencer_config_ = new ::rk::projects::durable_log::SequencerConfig(*from._impl_.sequencer_config_);
  }
  ::memcpy(&_impl_.version_id_, &from._impl_.version_id_,
    static_cast<size_t>(reinterpret_cast<char*>(&_impl_.previous_version_end_index_) -
    reinterpret_cast<char*>(&_impl_.version_id_)) + sizeof(_impl_.previous_version_end_index_));
  // @@protoc_insertion_point(copy_constructor:rk.projects.durable_log.MetadataConfig)
}

inline void MetadataConfig::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.replica_set_config_){arena}
    , decltype(_impl_.sequencer_config_){nullptr}
    , decltype(_impl_.version_id_){int64_t{0}}
    , decltype(_impl_.previous_version_id_){int64_t{0}}
    , decltype(_impl_.start_index_){int64_t{0}}
    , decltype(_impl_.end_index_){int64_t{0}}
    , decltype(_impl_.previous_version_end_index_){int64_t{0}}
    , /*decltype(_impl_._cached_size_)*/{}
  };
}

MetadataConfig::~MetadataConfig() {
  // @@protoc_insertion_point(destructor:rk.projects.durable_log.MetadataConfig)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void MetadataConfig::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.replica_set_config_.~RepeatedPtrField();
  if (this != internal_default_instance()) delete _impl_.sequencer_config_;
}

void MetadataConfig::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void MetadataConfig::Clear() {
// @@protoc_insertion_point(message_clear_start:rk.projects.durable_log.MetadataConfig)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.replica_set_config_.Clear();
  if (GetArenaForAllocation() == nullptr && _impl_.sequencer_config_ != nullptr) {
    delete _impl_.sequencer_config_;
  }
  _impl_.sequencer_config_ = nullptr;
  ::memset(&_impl_.version_id_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&_impl_.previous_version_end_index_) -
      reinterpret_cast<char*>(&_impl_.version_id_)) + sizeof(_impl_.previous_version_end_index_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* MetadataConfig::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // int64 version_id = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 8)) {
          _impl_.version_id_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // int64 previous_version_id = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 16)) {
          _impl_.previous_version_id_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // int64 start_index = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 24)) {
          _impl_.start_index_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // int64 end_index = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 32)) {
          _impl_.end_index_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // int64 previous_version_end_index = 5;
      case 5:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 40)) {
          _impl_.previous_version_end_index_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // .rk.projects.durable_log.SequencerConfig sequencer_config = 6;
      case 6:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 50)) {
          ptr = ctx->ParseMessage(_internal_mutable_sequencer_config(), ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // repeated .rk.projects.durable_log.ReplicaConfig replica_set_config = 7;
      case 7:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 58)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_replica_set_config(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<58>(ptr));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* MetadataConfig::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:rk.projects.durable_log.MetadataConfig)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // int64 version_id = 1;
  if (this->_internal_version_id() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteInt64ToArray(1, this->_internal_version_id(), target);
  }

  // int64 previous_version_id = 2;
  if (this->_internal_previous_version_id() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteInt64ToArray(2, this->_internal_previous_version_id(), target);
  }

  // int64 start_index = 3;
  if (this->_internal_start_index() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteInt64ToArray(3, this->_internal_start_index(), target);
  }

  // int64 end_index = 4;
  if (this->_internal_end_index() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteInt64ToArray(4, this->_internal_end_index(), target);
  }

  // int64 previous_version_end_index = 5;
  if (this->_internal_previous_version_end_index() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteInt64ToArray(5, this->_internal_previous_version_end_index(), target);
  }

  // .rk.projects.durable_log.SequencerConfig sequencer_config = 6;
  if (this->_internal_has_sequencer_config()) {
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
      InternalWriteMessage(6, _Internal::sequencer_config(this),
        _Internal::sequencer_config(this).GetCachedSize(), target, stream);
  }

  // repeated .rk.projects.durable_log.ReplicaConfig replica_set_config = 7;
  for (unsigned i = 0,
      n = static_cast<unsigned>(this->_internal_replica_set_config_size()); i < n; i++) {
    const auto& repfield = this->_internal_replica_set_config(i);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
        InternalWriteMessage(7, repfield, repfield.GetCachedSize(), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:rk.projects.durable_log.MetadataConfig)
  return target;
}

size_t MetadataConfig::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:rk.projects.durable_log.MetadataConfig)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .rk.projects.durable_log.ReplicaConfig replica_set_config = 7;
  total_size += 1UL * this->_internal_replica_set_config_size();
  for (const auto& msg : this->_impl_.replica_set_config_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  // .rk.projects.durable_log.SequencerConfig sequencer_config = 6;
  if (this->_internal_has_sequencer_config()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(
        *_impl_.sequencer_config_);
  }

  // int64 version_id = 1;
  if (this->_internal_version_id() != 0) {
    total_size += ::_pbi::WireFormatLite::Int64SizePlusOne(this->_internal_version_id());
  }

  // int64 previous_version_id = 2;
  if (this->_internal_previous_version_id() != 0) {
    total_size += ::_pbi::WireFormatLite::Int64SizePlusOne(this->_internal_previous_version_id());
  }

  // int64 start_index = 3;
  if (this->_internal_start_index() != 0) {
    total_size += ::_pbi::WireFormatLite::Int64SizePlusOne(this->_internal_start_index());
  }

  // int64 end_index = 4;
  if (this->_internal_end_index() != 0) {
    total_size += ::_pbi::WireFormatLite::Int64SizePlusOne(this->_internal_end_index());
  }

  // int64 previous_version_end_index = 5;
  if (this->_internal_previous_version_end_index() != 0) {
    total_size += ::_pbi::WireFormatLite::Int64SizePlusOne(this->_internal_previous_version_end_index());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData MetadataConfig::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    MetadataConfig::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*MetadataConfig::GetClassData() const { return &_class_data_; }


void MetadataConfig::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<MetadataConfig*>(&to_msg);
  auto& from = static_cast<const MetadataConfig&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:rk.projects.durable_log.MetadataConfig)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_impl_.replica_set_config_.MergeFrom(from._impl_.replica_set_config_);
  if (from._internal_has_sequencer_config()) {
    _this->_internal_mutable_sequencer_config()->::rk::projects::durable_log::SequencerConfig::MergeFrom(
        from._internal_sequencer_config());
  }
  if (from._internal_version_id() != 0) {
    _this->_internal_set_version_id(from._internal_version_id());
  }
  if (from._internal_previous_version_id() != 0) {
    _this->_internal_set_previous_version_id(from._internal_previous_version_id());
  }
  if (from._internal_start_index() != 0) {
    _this->_internal_set_start_index(from._internal_start_index());
  }
  if (from._internal_end_index() != 0) {
    _this->_internal_set_end_index(from._internal_end_index());
  }
  if (from._internal_previous_version_end_index() != 0) {
    _this->_internal_set_previous_version_end_index(from._internal_previous_version_end_index());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void MetadataConfig::CopyFrom(const MetadataConfig& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:rk.projects.durable_log.MetadataConfig)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool MetadataConfig::IsInitialized() const {
  return true;
}

void MetadataConfig::InternalSwap(MetadataConfig* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.replica_set_config_.InternalSwap(&other->_impl_.replica_set_config_);
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(MetadataConfig, _impl_.previous_version_end_index_)
      + sizeof(MetadataConfig::_impl_.previous_version_end_index_)
      - PROTOBUF_FIELD_OFFSET(MetadataConfig, _impl_.sequencer_config_)>(
          reinterpret_cast<char*>(&_impl_.sequencer_config_),
          reinterpret_cast<char*>(&other->_impl_.sequencer_config_));
}

::PROTOBUF_NAMESPACE_ID::Metadata MetadataConfig::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_MetadataConfig_2eproto_getter, &descriptor_table_MetadataConfig_2eproto_once,
      file_level_metadata_MetadataConfig_2eproto[2]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace durable_log
}  // namespace projects
}  // namespace rk
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::rk::projects::durable_log::SequencerConfig*
Arena::CreateMaybeMessage< ::rk::projects::durable_log::SequencerConfig >(Arena* arena) {
  return Arena::CreateMessageInternal< ::rk::projects::durable_log::SequencerConfig >(arena);
}
template<> PROTOBUF_NOINLINE ::rk::projects::durable_log::ReplicaConfig*
Arena::CreateMaybeMessage< ::rk::projects::durable_log::ReplicaConfig >(Arena* arena) {
  return Arena::CreateMessageInternal< ::rk::projects::durable_log::ReplicaConfig >(arena);
}
template<> PROTOBUF_NOINLINE ::rk::projects::durable_log::MetadataConfig*
Arena::CreateMaybeMessage< ::rk::projects::durable_log::MetadataConfig >(Arena* arena) {
  return Arena::CreateMessageInternal< ::rk::projects::durable_log::MetadataConfig >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
