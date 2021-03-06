#ifndef ENTT_CORE_TYPE_INFO_HPP
#define ENTT_CORE_TYPE_INFO_HPP


#include <string_view>
#include <type_traits>
#include <utility>
#include "../config/config.h"
#include "../core/attribute.h"
#include "hashed_string.hpp"
#include "fwd.hpp"


namespace entt {


/**
 * @cond TURN_OFF_DOXYGEN
 * Internal details not to be documented.
 */


namespace internal {


struct ENTT_API type_seq final {
    [[nodiscard]] static id_type next() ENTT_NOEXCEPT {
        static ENTT_MAYBE_ATOMIC(id_type) value{};
        return value++;
    }
};


template<typename Type>
[[nodiscard]] constexpr auto stripped_type_name() ENTT_NOEXCEPT {
#if defined ENTT_PRETTY_FUNCTION
    std::string_view pretty_function{ENTT_PRETTY_FUNCTION};
    auto first = pretty_function.find_first_not_of(' ', pretty_function.find_first_of(ENTT_PRETTY_FUNCTION_PREFIX)+1);
    auto value = pretty_function.substr(first, pretty_function.find_last_of(ENTT_PRETTY_FUNCTION_SUFFIX) - first);
    return value;
#else
    return std::string_view{""};
#endif
}


template<typename Type, auto = stripped_type_name<Type>().find_first_of('.')>
[[nodiscard]] static constexpr std::string_view type_name(int) ENTT_NOEXCEPT {
    constexpr auto value = stripped_type_name<Type>();
    return value;
}


template<typename Type>
[[nodiscard]] static std::string_view type_name(char) ENTT_NOEXCEPT {
    static const auto value = stripped_type_name<Type>();
    return value;
}


template<typename Type, auto = stripped_type_name<Type>().find_first_of('.')>
[[nodiscard]] static constexpr id_type type_hash(int) ENTT_NOEXCEPT {
    constexpr auto stripped = stripped_type_name<Type>();
    constexpr auto value = hashed_string::value(stripped.data(), stripped.size());
    return value;
}


template<typename Type>
[[nodiscard]] static id_type type_hash(char) ENTT_NOEXCEPT {
    static const auto value = [](const auto stripped) {
        return hashed_string::value(stripped.data(), stripped.size());
    }(stripped_type_name<Type>());
    return value;
}


}


/**
 * Internal details not to be documented.
 * @endcond
 */


/**
 * @brief Type sequential identifier.
 * @tparam Type Type for which to generate a sequential identifier.
 */
template<typename Type, typename = void>
struct ENTT_API type_seq final {
    /**
     * @brief Returns the sequential identifier of a given type.
     * @return The sequential identifier of a given type.
     */
    [[nodiscard]] static id_type value() ENTT_NOEXCEPT {
        static const id_type value = internal::type_seq::next();
        return value;
    }

    /*! @copydoc value */
    [[nodiscard]] constexpr operator id_type() const ENTT_NOEXCEPT { return value(); }
};


/**
 * @brief Type hash.
 * @tparam Type Type for which to generate a hash value.
 */
template<typename Type, typename = void>
struct type_hash final {
    /**
     * @brief Returns the numeric representation of a given type.
     * @return The numeric representation of the given type.
     */
#if defined ENTT_PRETTY_FUNCTION
    [[nodiscard]] static constexpr id_type value() ENTT_NOEXCEPT {
        return internal::type_hash<Type>(0);
#else
    [[nodiscard]] static constexpr id_type value() ENTT_NOEXCEPT {
        return type_seq<Type>::value();
#endif
    }

    /*! @copydoc value */
    [[nodiscard]] constexpr operator id_type() const ENTT_NOEXCEPT { return value(); }
};


/**
 * @brief Type name.
 * @tparam Type Type for which to generate a name.
 */
template<typename Type, typename = void>
struct type_name final {
    /**
     * @brief Returns the name of a given type.
     * @return The name of the given type.
     */
    [[nodiscard]] static constexpr std::string_view value() ENTT_NOEXCEPT {
        return internal::type_name<Type>(0);
    }

    /*! @copydoc value */
    [[nodiscard]] constexpr operator std::string_view() const ENTT_NOEXCEPT { return value(); }
};


/*! @brief Implementation specific information about a type. */
class type_info final {
    template<typename>
    friend type_info type_id() ENTT_NOEXCEPT;

    enum class operation { SEQ, HASH, NAME };

    using vtable_type = id_type(const operation, void *);

    template<typename Type>
    static id_type basic_vtable(const operation op, void *to) {
        static_assert(std::is_same_v<std::remove_reference_t<std::remove_const_t<Type>>, Type>, "Invalid type");

        switch(op) {
        case operation::SEQ:
            return type_seq<Type>::value();
            break;
        case operation::HASH:
            return type_hash<Type>::value();
            break;
        case operation::NAME:
            *static_cast<std::string_view *>(to) = type_name<Type>::value();
            break;
        }

        return {};
    }

public:
    /*! @brief Default constructor. */
    constexpr type_info() ENTT_NOEXCEPT
        : vtable{}
    {}

    /*! @brief Default copy constructor. */
    type_info(const type_info &) ENTT_NOEXCEPT = default;
    /*! @brief Default move constructor. */
    type_info(type_info &&) ENTT_NOEXCEPT = default;

    /**
     * @brief Creates a type info object for a given type.
     * @tparam Type Type for which to generate a type info object.
     */
    template<typename Type>
    type_info(std::in_place_type_t<Type>) ENTT_NOEXCEPT
        : vtable{&basic_vtable<Type>}
    {}

    /**
     * @brief Default copy assignment operator.
     * @return This type info object.
     */
    type_info & operator=(const type_info &) ENTT_NOEXCEPT = default;

    /**
     * @brief Default move assignment operator.
     * @return This type info object.
     */
    type_info & operator=(type_info &&) ENTT_NOEXCEPT = default;

    /**
     * @brief Checks if a type info object is properly initialized.
     * @return True if the object is properly initialized, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const ENTT_NOEXCEPT {
        return vtable != nullptr;
    }

    /**
     * @brief Type sequential identifier.
     * @return Type sequential identifier.
     */
    [[nodiscard]] id_type seq() const ENTT_NOEXCEPT {
        return vtable ? vtable(operation::SEQ, nullptr) : id_type{};
    }

    /**
     * @brief Type hash.
     * @return Type hash.
     */
    [[nodiscard]] id_type hash() const ENTT_NOEXCEPT {
        return vtable ? vtable(operation::HASH, nullptr) : id_type{};
    }

    /**
     * @brief Type name.
     * @return Type name.
     */
    [[nodiscard]] std::string_view name() const ENTT_NOEXCEPT {
        std::string_view value{};

        if(vtable) {
            vtable(operation::NAME, &value);
        }

        return value;
    }

    /**
     * @brief Compares the contents of two type info objects.
     * @param other Object with which to compare.
     * @return False if the two contents differ, true otherwise.
     */
    [[nodiscard]] bool operator==(const type_info &other) const ENTT_NOEXCEPT {
        return hash() == other.hash();
    }

private:
    vtable_type *vtable;
};


/**
 * @brief Compares the contents of two type info objects.
 * @param lhs A type info object.
 * @param rhs A type info object.
 * @return True if the two contents differ, false otherwise.
 */
[[nodiscard]] inline bool operator!=(const type_info &lhs, const type_info &rhs) ENTT_NOEXCEPT {
    return !(lhs == rhs);
}


/**
 * @brief Returns the type info object for a given type.
 * @tparam Type Type for which to generate a type info object.
 * @return The type info object for the given type.
 */
template<typename Type>
[[nodiscard]] type_info type_id() ENTT_NOEXCEPT {
    return type_info{std::in_place_type<std::remove_cv_t<std::remove_reference_t<Type>>>};
}


}


#endif
