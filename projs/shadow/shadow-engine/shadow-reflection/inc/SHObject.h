#pragma once
#include <string>
#include <typeinfo>

#include "exports.h"

namespace ShadowEngine {

	/**
	 * \brief This is the base class for every class in the Engine that uses runtime reflection.

	 * Currently it provides a runtime TypeID and TypeName witch can be accesed as static and as class memebers.
	 * The ID is a int type number witch is generated incramently, on the first call to get a type.

	 * Each class that inherits from this or it's parent inheris form it must implement the
		SHObject::GetType and SHObject::GetTypeId methodes and make it's own static methodes.
		To make it easier a standard implementation of these can be used with the SHObject_Base() macro
		witch implements all of these functions. It uses the typeid().name of the class.

	 */
	class SHObject
	{
    public:
		/**
		 * \brief Generates a new UID for each call
		 * \return the next Unique ID that was just generated
		 */
		API static uint64_t GenerateId() noexcept;

	public:
		/**
		 * \brief Returns the top level class type name of the object
		 * \return The class Class name as a string
		 */
		virtual const std::string& GetType() const = 0;
		/**
		 * \brief Gets the top level type ID
		 * \return UID of the class
		 */
		virtual const uint64_t GetTypeId() const = 0;

		virtual ~SHObject() = default;
	};


	/**
	 * \brief Macro to make the override functions of SHObject. This should be added in each derived class
	 * \param type The type of the class
	 */
#define SHObject_Base(type)	\
public: \
	static const std::string& Type();				 \
	static uint64_t TypeId();						 \
	const std::string& GetType() const override		{ return Type();  } \
	const uint64_t GetTypeId() const override		{ return  type::TypeId(); } \
private:

#define SHObject_Base_Impl(type)	\
	const std::string& type::Type()				{ static const std::string t = typeid(type).name(); return t; } \
	uint64_t type::TypeId()						{ static const uint64_t id = GenerateId(); return id; }
}