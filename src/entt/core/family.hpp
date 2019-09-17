#ifndef ENTT_CORE_FAMILY_HPP
#define ENTT_CORE_FAMILY_HPP


#include <type_traits>
#include "../config/config.h"


namespace entt {


/**
 * @brief Dynamic identifier generator.
 *
 * Utility class template that can be used to assign unique identifiers to types
 * at runtime. Use different specializations to create separate sets of
 * identifiers.
 */
template<typename...>
class family {

public:
	/*! @brief Unsigned integer type. */
	using family_type = ENTT_ID_TYPE;


private:
    inline static ENTT_MAYBE_ATOMIC(ENTT_ID_TYPE) identifier;

	template<typename...>
	static family_type family_id() {
		static const family_type value = identifier++;
		return value;
	}

public:

	template<typename... Type>
	inline static family_type type() {
		return family_id<std::decay_t<Type>...>();
	}
};

template<typename... Types>
ENTT_MAYBE_ATOMIC(ENTT_ID_TYPE) family<Types...>::identifier{};

}


#endif // ENTT_CORE_FAMILY_HPP
