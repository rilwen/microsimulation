#ifndef __AVERISERA_PRINTABLE_HPP
#define __AVERISERA_PRINTABLE_HPP

#include <cstdint>
#include <iosfwd>
#include <string>

namespace averisera {
    /** Interface to every object which can print itself */
    class Printable {
    public:
        virtual ~Printable();

        /** Print the object as text. Do not use ';' character. */
        virtual void print(std::ostream& os) const = 0;

		std::string as_string() const;
    };

    inline std::ostream& operator<<(std::ostream& os, const Printable& printable) {
        printable.print(os);
        return os;
    }

    /** Return human-readable type name */
    template <class T> const char* print_type_name();

    template <> inline const char* print_type_name<double>() {
        return "double";
    }

	template <> inline const char* print_type_name<float>() {
		return "float";
	}

    // Integer types have fixed width so that when we write data to disk from 64-bit library and read them into a 32-bit library, we get no overflow.

    template <> inline const char* print_type_name<int64_t>() {
        return "int64";
    }

    template <> inline const char* print_type_name<uint64_t>() {
        return "uint64";
    }

    template <> inline const char* print_type_name<int32_t>() {
        return "int32";
    }

    template <> inline const char* print_type_name<uint32_t>() {
        return "uint32";
    }

    template <> inline const char* print_type_name<int16_t>() {
        return "int16";
    }

    template <> inline const char* print_type_name<uint16_t>() {
        return "uint16";
    }


    template <> inline const char* print_type_name<int8_t>() {
        return "int8";
    }

    template <> inline const char* print_type_name<uint8_t>() {
        return "uint8";
    }       
}

#endif // __AVERISERA_PRINTABLE_HPP
