#ifndef ENTT_CORE_FAMILY_HPP
#define ENTT_CORE_FAMILY_HPP


#include<type_traits>
#include<cstddef>
#include<atomic>


namespace entt {


/**
 * @brief Dynamic identifier generator.
 *
 * Utility class template that can be used to assign unique identifiers to types
 * at runtime. Use different specializations to create separate sets of
 * identifiers.
 */
template<typename...>
class Family {
public:
	/*! @brief Unsigned integer type. */
	using family_type = std::uint32_t;

private:
    static std::atomic<family_type>& identifier() noexcept {
		static std::atomic<family_type> id{0};
		return id;
	}

    template<typename...>
    static family_type family() noexcept {
        static const family_type value = identifier().fetch_add(1);
        return value;
    }

public:
    /**
     * @brief Returns an unique identifier for the given type.
     * @return Statically generated unique identifier for the given type.
     */
    template<typename... Type>
    inline static family_type type() noexcept {
        return family<std::decay_t<Type>...>();
    }
	
	static family_type max() noexcept {
		return identifier();
	}
};


}


#endif // ENTT_CORE_FAMILY_HPP
