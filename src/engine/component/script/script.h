#ifndef PRT3_SCRIPT_H
#define PRT3_SCRIPT_H

#include "src/engine/scene/node.h"
#include "src/engine/scene/signal.h"
#include "src/util/serialization_util.h"
#include "src/util/fixed_string.h"
#include "src/util/uuid.h"

#include <cstdint>
#include <unordered_map>
#include <iostream>

namespace prt3 {

typedef unsigned int ScriptID;
static constexpr ScriptID NO_SCRIPT = -1;

class Scene;
class Script {
public:
    using TScriptDeserializer = Script *(*)(std::istream &, Scene &, NodeID);
    using TScriptInstantiator = Script *(*)(Scene &, NodeID);

    explicit Script(Scene & scene, NodeID node_id);
    virtual ~Script() {}

    virtual char const * name() = 0;

    virtual void on_start(Scene &) {}
    virtual void on_init(Scene &) {}
    virtual void on_late_init(Scene &) {}
    virtual void on_update(Scene & /*scene*/, float /*delta_time*/) {}
    virtual void on_late_update(Scene & /*scene*/, float /*delta_time*/) {}
    virtual void on_signal(
        Scene & /*scene*/,
        SignalString const & /*signal*/,
        void * /*data*/
    ) {}

    // Only called for autoloaded scripts
    virtual void save_state(Scene const & /*scene*/, std::ostream & /*out*/) const {}
    // Only called for autoloaded scripts
    virtual void restore_state(Scene & /*scene*/, std::istream & /*in*/) {}

    enum class FieldType {
        uint8,
        uint16,
        uint32,
        uint64,
        int8,
        int16,
        int32,
        int64,
        f32,
        f64,
        boolean
    };

    union FieldValue {
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        int8_t   i8;
        int16_t  i16;
        int32_t  i32;
        int64_t  i64;
        float    f32;
        double   f64;
        bool     boolean;
    };

    template<typename T>
    static FieldType type_to_field_type();

    template<>
    FieldType type_to_field_type<uint8_t>()  { return FieldType::uint8;   }
    template<>
    FieldType type_to_field_type<uint16_t>() { return FieldType::uint16;  }
    template<>
    FieldType type_to_field_type<uint32_t>() { return FieldType::uint32;  }
    template<>
    FieldType type_to_field_type<uint64_t>() { return FieldType::uint64;  }
    template<>
    FieldType type_to_field_type<int8_t>()   { return FieldType::int8;    }
    template<>
    FieldType type_to_field_type<int16_t>()  { return FieldType::int16;   }
    template<>
    FieldType type_to_field_type<int32_t>()  { return FieldType::int32;   }
    template<>
    FieldType type_to_field_type<int64_t>()  { return FieldType::int64;   }
    template<>
    FieldType type_to_field_type<float>()    { return FieldType::f32;     }
    template<>
    FieldType type_to_field_type<double>()   { return FieldType::f64;     }
    template<>
    FieldType type_to_field_type<bool>()     { return FieldType::boolean; }

    struct SerializedField {
        FieldType type;
        union {
            uint8_t  * (*u8)(void*);
            uint16_t * (*u16)(void*);
            uint32_t * (*u32)(void*);
            uint64_t * (*u64)(void*);
            int8_t   * (*i8)(void*);
            int16_t  * (*i16)(void*);
            int32_t  * (*i32)(void*);
            int64_t  * (*i64)(void*);
            float    * (*f32)(void*);
            double   * (*f64)(void*);
            bool     * (*boolean)(void*);
        } getter;
        char const * name;

        template<typename T>
        T & get(void * p) const { return *(*u_getter<T>())(p); }

        template<typename T>
        T * (*const*u_getter()const)(void*);
        template<typename T>
        T * (**u_getter())(void*);

        template<> uint8_t  * (*const*u_getter<uint8_t>()const)(void*)  { return &getter.u8; }
        template<> uint16_t * (*const*u_getter<uint16_t>()const)(void*) { return &getter.u16; }
        template<> uint32_t * (*const*u_getter<uint32_t>()const)(void*) { return &getter.u32; }
        template<> uint64_t * (*const*u_getter<uint64_t>()const)(void*) { return &getter.u64; }
        template<> int8_t   * (*const*u_getter<int8_t>()const)(void*)   { return &getter.i8; }
        template<> int16_t  * (*const*u_getter<int16_t>()const)(void*)  { return &getter.i16; }
        template<> int32_t  * (*const*u_getter<int32_t>()const)(void*)  { return &getter.i32; }
        template<> int64_t  * (*const*u_getter<int64_t>()const)(void*)  { return &getter.i64; }
        template<> float    * (*const*u_getter<float>()const)(void*)    { return &getter.f32; }
        template<> double   * (*const*u_getter<double>()const)(void*)   { return &getter.f64; }
        template<> bool     * (*const*u_getter<bool>()const)(void*)     { return &getter.boolean; }

        template<> uint8_t  * (**u_getter<uint8_t>())(void*)  { return &getter.u8; }
        template<> uint16_t * (**u_getter<uint16_t>())(void*) { return &getter.u16; }
        template<> uint32_t * (**u_getter<uint32_t>())(void*) { return &getter.u32; }
        template<> uint64_t * (**u_getter<uint64_t>())(void*) { return &getter.u64; }
        template<> int8_t   * (**u_getter<int8_t>())(void*)   { return &getter.i8; }
        template<> int16_t  * (**u_getter<int16_t>())(void*)  { return &getter.i16; }
        template<> int32_t  * (**u_getter<int32_t>())(void*)  { return &getter.i32; }
        template<> int64_t  * (**u_getter<int64_t>())(void*)  { return &getter.i64; }
        template<> float    * (**u_getter<float>())(void*)    { return &getter.f32; }
        template<> double   * (**u_getter<double>())(void*)   { return &getter.f64; }
        template<> bool     * (**u_getter<bool>())(void*)     { return &getter.boolean; }
    };

    virtual void serialize(std::ostream & out) const;

    virtual Script * copy() const = 0;

    static Script * deserialize(
        std::istream & in,
        Scene & scene,
        NodeID node_id
    );

    static Script * instantiate(
        UUID uuid,
        Scene & scene,
        NodeID node_id
    );

    static std::vector<SerializedField> const & get_serialized_fields(UUID uuid)
    { return *s_serialized_fields->at(uuid); }

    virtual UUID uuid() const = 0;

    static std::unordered_map<UUID, char const *> const & script_names()
    { return *s_script_names; }

    static char const * get_script_name(UUID uuid)
    { return s_script_names->at(uuid); }

    NodeID node_id() const { return m_node_id; }
    Node & get_node(Scene & scene);
protected:

    bool set_tag(Scene & scene, NodeTag const & tag);

    static std::unordered_map<UUID, TScriptDeserializer> * s_deserializers;
    static std::unordered_map<UUID, TScriptInstantiator> * s_instantiators;
    static std::unordered_map<UUID, std::vector<SerializedField> const *> * s_serialized_fields;
    static std::unordered_map<UUID, char const *> * s_script_names;

    static bool Register(
        UUID uuid,
        char const * name,
        TScriptDeserializer deserializer,
        TScriptInstantiator instantiator,
        std::vector<SerializedField> const * serialized_fields
    );

private:
    NodeID m_node_id;
};

#define REGISTER_SERIALIZED_FIELD(field)\
    static char const * str_##field = #field;\
    typedef std::remove_reference<decltype(*dummy)>::type ClassT;\
    typedef decltype(field) T##field;\
    static T##field * (*get_##field)(void*) = [](void*p) { return &reinterpret_cast<ClassT*>(p)->field; };\
    fields.push_back({});\
    fields.back().type = type_to_field_type<T##field>();\
    *fields.back().u_getter<T##field>() = get_##field;\
    fields.back().name = str_##field;\

// The var args are serialized fields
#define REGISTER_SCRIPT_BEGIN(class_name, display_name, serialization_uuid)\
public:\
    virtual char const * name() { return #display_name; };\
    virtual Script * copy() const { return new class_name(*this); }\
    virtual UUID uuid() const {\
        return serialization_uuid##ull;\
    }\
    static constexpr UUID s_uuid = serialization_uuid##ull;\
protected:\
    static Script * deserialize(\
        std::istream & in,\
        Scene & scene,\
        NodeID node_id\
    ) {\
        return new class_name(in, scene, node_id);\
    }\
    static Script * new_instance(\
        Scene & scene,\
        NodeID node_id\
    ) {\
        return new class_name(scene, node_id);\
    }\
    static inline std::vector<Script::SerializedField> const * serialized_fields() {\
        return inner_serialized_fields();\
    }\
    inline static bool s_registered =\
        Script::Register(\
            serialization_uuid##ull,\
            #display_name,\
            class_name::deserialize,\
            class_name::new_instance,\
            class_name::serialized_fields()\
        );\
    static std::vector<Script::SerializedField> const * inner_serialized_fields() {\
        static std::vector<Script::SerializedField> fields;\
        [[maybe_unused]] class_name *dummy;\

#define REGISTER_SCRIPT_END()\
        return &fields;\
    }\

#define REGISTER_SCRIPT(class_name, display_name, serialization_uuid)\
REGISTER_SCRIPT_BEGIN(class_name, display_name, serialization_uuid)\
REGISTER_SCRIPT_END()\

} // namespace prt3

#endif
