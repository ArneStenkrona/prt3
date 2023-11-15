#include "script.h"

#include "src/engine/scene/scene.h"
#include "src/engine/component/script/script_include.h"

using namespace prt3;

Script::Script(Scene &, NodeID node_id)
 : m_node_id{node_id} {
}

Node & Script::get_node(Scene & scene) { return scene.get_node(m_node_id); }

bool Script::set_tag(Scene & scene, NodeTag const & tag) {
    return scene.set_node_tag(tag, m_node_id);
}

std::unordered_map<UUID, Script::TScriptDeserializer> *
    Script::s_deserializers = nullptr;

std::unordered_map<UUID, Script::TScriptInstantiator> *
    Script::s_instantiators = nullptr;

std::unordered_map<UUID, std::vector<Script::SerializedField> const *> *
    Script::s_serialized_fields = nullptr;

std::unordered_map<UUID, char const *> *
    Script::s_script_names = nullptr;

void Script::serialize(std::ostream & out) const {
    write_stream(out, uuid());

    serialize_extension(out);

    auto const & fields = Script::get_serialized_fields(uuid());
    write_stream(out, fields.size());

    size_t field_index = 0;
    for (SerializedField const & field : fields) {
        write_stream(out, field_index);
        write_stream(out, field.type);

        thread_local FixedString<64> fixed_name;
        fixed_name = field.name;

        size_t name_hash = std::hash<prt3::FixedString<64> >{}(fixed_name);
        write_stream(out, name_hash);

        void * sptr = reinterpret_cast<void*>(const_cast<Script*>(this));

        switch(field.type) {
            case FieldType::uint8: write_stream(out, field.get<uint8_t>(sptr)); break;
            case FieldType::uint16: write_stream(out, field.get<uint16_t>(sptr)); break;
            case FieldType::uint32: write_stream(out, field.get<uint32_t>(sptr)); break;
            case FieldType::uint64: write_stream(out, field.get<uint64_t>(sptr)); break;
            case FieldType::int8: write_stream(out, field.get<int8_t>(sptr)); break;
            case FieldType::int16: write_stream(out, field.get<int16_t>(sptr)); break;
            case FieldType::int32: write_stream(out, field.get<int32_t>(sptr)); break;
            case FieldType::int64: write_stream(out, field.get<int64_t>(sptr)); break;
            case FieldType::f32: write_stream(out, field.get<float>(sptr)); break;
            case FieldType::f64: write_stream(out, field.get<double>(sptr)); break;
            case FieldType::boolean: write_stream(out, field.get<bool>(sptr)); break;
        }
        ++field_index;
    }
}

Script * Script::deserialize(
    std::istream & in,
    Scene & scene,
    NodeID node_id
) {
    UUID uuid;
    read_stream(in, uuid);

    Script * script = s_deserializers->at(uuid)(in, scene, node_id);

    auto const & fields = Script::get_serialized_fields(uuid);

    size_t n_fields = 0;
    read_stream(in, n_fields);

    for (size_t i = 0; i < n_fields; ++i) {
        size_t field_index;
        read_stream(in, field_index);

        FieldType field_type;
        read_stream(in, field_type);

        size_t name_hash;
        read_stream(in, name_hash);

        FieldValue val;
        switch(field_type) {
            case FieldType::uint8: read_stream(in, val.u8); break;
            case FieldType::uint16: read_stream(in, val.u16); break;
            case FieldType::uint32: read_stream(in, val.u32); break;
            case FieldType::uint64: read_stream(in, val.u64); break;
            case FieldType::int8: read_stream(in, val.i8); break;
            case FieldType::int16: read_stream(in, val.i16); break;
            case FieldType::int32: read_stream(in, val.i32); break;
            case FieldType::int64: read_stream(in, val.i64); break;
            case FieldType::f32: read_stream(in, val.f32); break;
            case FieldType::f64: read_stream(in, val.f64); break;
            case FieldType::boolean: read_stream(in, val.boolean); break;
        }

        SerializedField const & field = fields[field_index];

        if (field_index >= fields.size()) continue;
        if (field_type != field.type) continue;

        thread_local FixedString<64> fixed_name;
        fixed_name = field.name;
        size_t hash = std::hash<prt3::FixedString<64> >{}(fixed_name);
        if (name_hash != hash) continue;
        void * sptr = reinterpret_cast<void*>(script);
        switch(field.type) {
            case Script::FieldType::uint8: field.get<uint8_t>(sptr) = val.u8; break;
            case Script::FieldType::uint16: field.get<uint16_t>(sptr) = val.u16; break;
            case Script::FieldType::uint32: field.get<uint32_t>(sptr) = val.u32; break;
            case Script::FieldType::uint64: field.get<uint64_t>(sptr) = val.u64; break;
            case Script::FieldType::int8: field.get<int8_t>(sptr) = val.i8; break;
            case Script::FieldType::int16: field.get<int16_t>(sptr) = val.i16; break;
            case Script::FieldType::int32: field.get<int32_t>(sptr) = val.i32; break;
            case Script::FieldType::int64: field.get<int64_t>(sptr) = val.i64; break;
            case Script::FieldType::f32: field.get<float>(sptr)   = val.f32; break;
            case Script::FieldType::f64: field.get<double>(sptr)  = val.f64; break;
            case Script::FieldType::boolean: field.get<bool>(sptr) = val.boolean; break;
        }
    }

    return script;
}

Script * Script::instantiate(
    UUID uuid,
    Scene & scene,
    NodeID node_id
) {
    if (auto it = s_instantiators->find(uuid); it != s_instantiators->end()) {
        return it->second(scene, node_id);
    }

    return nullptr;
}

bool Script::Register(
    UUID uuid,
    char const * name,
    Script::TScriptDeserializer deserializer,
    Script::TScriptInstantiator instantiator,
    std::vector<SerializedField> const * serialized_fields
) {
    if (s_deserializers == nullptr) {
        static std::unique_ptr<
            std::unordered_map<UUID, Script::TScriptDeserializer>
        > deserializers{
            new std::unordered_map<UUID, Script::TScriptDeserializer>()
        };
        static std::unique_ptr<
            std::unordered_map<UUID, Script::TScriptInstantiator>
        > instantiators{
            new std::unordered_map<UUID, Script::TScriptInstantiator>()
        };
        static std::unique_ptr<
            std::unordered_map<UUID, std::vector<Script::SerializedField> const *>
        > serialized_fields_map{
            new std::unordered_map<UUID, std::vector<Script::SerializedField> const *>()
        };
        static std::unique_ptr<std::unordered_map<UUID, char const *>
        > names{
            new std::unordered_map<UUID, char const *>()
        };

        s_deserializers = deserializers.get();
        s_instantiators = instantiators.get();
        s_serialized_fields = serialized_fields_map.get();
        s_script_names = names.get();
    }

    if (s_deserializers->find(uuid) != s_deserializers->end()) {
        return false;
    }
    (*s_script_names)[uuid] = name;
    (*s_deserializers)[uuid] = deserializer;
    (*s_serialized_fields)[uuid] = serialized_fields;
    (*s_instantiators)[uuid] = instantiator;

    return true;
}

size_t Script::get_serialized_field_index(char const * name) {
    size_t index = 0;
    for (SerializedField const & f : Script::get_serialized_fields(uuid())) {
        if (strcmp(f.name, name) == 0) return index;
        ++index;
    }

    return static_cast<size_t>(-1);
}

void Script::set_serialized_field_value(size_t field_index, FieldValue val) {
    void * sptr = reinterpret_cast<void*>(this);

    auto const & fields = Script::get_serialized_fields(uuid());
    SerializedField const & field = fields[field_index];

    switch(field.type) {
        case FieldType::uint8:   field.get<uint8_t>(sptr)  = val.u8;      break;
        case FieldType::uint16:  field.get<uint16_t>(sptr) = val.u16;     break;
        case FieldType::uint32:  field.get<uint32_t>(sptr) = val.u32;     break;
        case FieldType::uint64:  field.get<uint64_t>(sptr) = val.u64;     break;
        case FieldType::int8:    field.get<int8_t>(sptr)   = val.i8;      break;
        case FieldType::int16:   field.get<int16_t>(sptr)  = val.i16;     break;
        case FieldType::int32:   field.get<int32_t>(sptr)  = val.i32;     break;
        case FieldType::int64:   field.get<int64_t>(sptr)  = val.i64;     break;
        case FieldType::f32:     field.get<float>(sptr)    = val.f32;     break;
        case FieldType::f64:     field.get<double>(sptr)   = val.f64;     break;
        case FieldType::boolean: field.get<bool>(sptr)     = val.boolean; break;
    }
}

Script::FieldValue Script::get_serialized_field_value(size_t field_index) {
    void * sptr = reinterpret_cast<void*>(this);

    auto const & fields = Script::get_serialized_fields(uuid());
    SerializedField const & field = fields[field_index];

    FieldValue val;

    switch(field.type) {
        case FieldType::uint8:   val.u8      = field.get<uint8_t>(sptr);  break;
        case FieldType::uint16:  val.u16     = field.get<uint16_t>(sptr); break;
        case FieldType::uint32:  val.u32     = field.get<uint32_t>(sptr); break;
        case FieldType::uint64:  val.u64     = field.get<uint64_t>(sptr); break;
        case FieldType::int8:    val.i8      = field.get<int8_t>(sptr);   break;
        case FieldType::int16:   val.i16     = field.get<int16_t>(sptr);  break;
        case FieldType::int32:   val.i32     = field.get<int32_t>(sptr);  break;
        case FieldType::int64:   val.i64     = field.get<int64_t>(sptr);  break;
        case FieldType::f32:     val.f32     = field.get<float>(sptr);    break;
        case FieldType::f64:     val.f64     = field.get<double>(sptr);   break;
        case FieldType::boolean: val.boolean = field.get<bool>(sptr);     break;
    }

    return val;
}
