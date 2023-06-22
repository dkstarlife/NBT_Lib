#pragma once
#include <unordered_map>
#include <memory_resource>
#include <string>
#include <bit>
#include <sstream>

#include "NBT_LibUtil.h"

//https://wiki.vg/NBT
//https://minecraft.fandom.com/wiki/NBT_format

namespace NBT_Lib {
	using std::byte;

	enum class TagID {
		End			= 0u, 
		Byte		= 1u, 
		Short		= 2u, 
		Int			= 3u, 
		Long		= 4u, 
		Float		= 5u, 
		Double		= 6u, 
		Byte_Array	= 7u, 
		String		= 8u, 
		List		= 9u,  //0x09
		Compound	= 10u, //0x0a
		Int_Array	= 11u, //0x0b
		Long_Array	= 12u  //0x0c
	};
	constexpr std::string TagIDToString(TagID id) {
		using enum TagID;
		switch (id) {
		case End:
			return "TAG_End";
		case Byte:
			return "TAG_Byte";
		case Short:
			return "TAG_Short";
		case Int:
			return "TAG_Int";
		case Long:
			return "TAG_Long";
		case Float:
			return "TAG_Float";
		case Double:
			return "TAG_Double";
		case Byte_Array:
			return "TAG_Byte_Array";
		case String:
			return "TAG_String";
		case List:
			return "TAG_List";
		case Compound:
			return "TAG_Compound";
		case Int_Array:
			return "TAG_Int_Array";
		case Long_Array:
			return "TAG_Long_Array";
		default:
			return "UNKNOWN_TAG";
		}
	}

	void inline addTabsToStringStream(std::stringstream& ss, uint8_t tabDepth) {
		while (tabDepth > 0u) {
			//ss << '\t';
			ss << "   ";
			--tabDepth;
		}
	}

	struct NBT_TagBase {
		TagID id;
		std::pmr::string name;

		constexpr NBT_TagBase(TagID id, const std::pmr::string& name, std::pmr::memory_resource* memRes) : id{ id }, name{ name, memRes}{

		}

		void addTagHeaderToBinaryStream(BinaryStream& bstream) const {
			byte header[1u + sizeof(int16_t)];
			header[0] = static_cast<byte>(static_cast<int8_t>(id));
			if (id == TagID::End) {
				bstream.pushbackData(header, 1u);
				return;
			}

			int16_t flippedNameLength{ byteswap(static_cast<int16_t>(name.size())) };
			memcpy(header+1u, &flippedNameLength, sizeof(int16_t));
			bstream.pushbackData(header, sizeof(header));

			bstream.pushbackData(name.data(), name.size());
		}

		void virtual addTagToBinaryStream(BinaryStream& bstream) const = 0;
	};

	//Create a deep copy of a tag.
	NBT_TagBase* copyTag(NBT_TagBase* tagPtr, std::pmr::memory_resource* memRes);

	struct End_Tag : public NBT_TagBase {
		End_Tag(std::pmr::memory_resource* memRes) : NBT_TagBase(TagID::End, "", memRes) {

		}
		//Copy constructor
		constexpr End_Tag(const End_Tag& copyFrom)
			: NBT_TagBase(TagID::End, { copyFrom.name, copyFrom.name.get_allocator() }, copyFrom.name.get_allocator().resource()){
		}
		//Move constructor
		End_Tag(End_Tag&& moveFrom) noexcept
			: NBT_TagBase(TagID::End, { std::move(moveFrom.name), moveFrom.name.get_allocator() }, moveFrom.name.get_allocator().resource()) {
		}
		//Assignment copy
		//End_Tag& operator=(const End_Tag& copyFrom) = default;
		//Assignment move
		//End_Tag& operator=(End_Tag&& moveFrom) noexcept = default;

		void addTagToBinaryStream(BinaryStream& bstream) const override {
		}

		void addToStringStream(std::stringstream& ss, uint8_t tabDepth) const {
			addTabsToStringStream(ss, tabDepth);
			ss << "TAG_End";
		}
	};

	template<typename valueType, TagID tag_id>
	struct NumberType_Tag : public NBT_TagBase {
		valueType value;

		NumberType_Tag(const decltype(name)& name, valueType value, std::pmr::memory_resource* memRes) noexcept
			: NBT_TagBase(tag_id, name, memRes), value{ value } {
		}
		//Copy constructor
		NumberType_Tag(const NumberType_Tag& copyFrom)
			: NBT_TagBase(tag_id, { copyFrom.name, copyFrom.name.get_allocator() }, copyFrom.name.get_allocator().resource())
			, value{ copyFrom.value }{
		}
		//Move constructor
		NumberType_Tag(NumberType_Tag&& moveFrom) noexcept
			: NBT_TagBase(tag_id, { std::move(moveFrom.name), moveFrom.name.get_allocator() }, moveFrom.name.get_allocator().resource())
			, value{ moveFrom.value } {
		}

		static NumberType_Tag fromRawData(const decltype(name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes){

			if (maxReadLength < sizeof(valueType))
				throw std::out_of_range("Data ran out while creating " + TagIDToString(tag_id) + ": " + std::string{name});

			out_bytesRead = sizeof(valueType);
			return NumberType_Tag<valueType, tag_id>(name, copyAndFlipBytes<valueType>(dataPtr), memRes);
		}

		void addTagToBinaryStream(BinaryStream& bstream) const override {
			const valueType flippedData{ byteswap(value) };
			bstream.pushbackData(&flippedData, sizeof(flippedData));
		}

		void addToStringStream(std::stringstream& ss, uint8_t tabDepth) const {
			addTabsToStringStream(ss, tabDepth);
			ss << TagIDToString(id) << ": " << name << " = ";
			if (sizeof(valueType) == sizeof(int8_t))
				ss << int32_t(value);
			else
				ss << value;
		}
	};


	using Byte_Tag = NumberType_Tag<int8_t, TagID::Byte>;
	using Short_Tag = NumberType_Tag<int16_t, TagID::Short>;
	using Int_Tag = NumberType_Tag<int32_t, TagID::Int>;
	using Long_Tag = NumberType_Tag<int64_t, TagID::Long>;
	
	using Float_Tag = NumberType_Tag<float, TagID::Float>;
	using Double_Tag = NumberType_Tag<double, TagID::Double>;

	template<typename valueType, TagID tag_id>
	struct ArrayType_Tag : public NBT_TagBase {
		std::pmr::vector<valueType> values;
		ArrayType_Tag(const decltype(name)& name, decltype(values) values, std::pmr::memory_resource* memRes)
			: NBT_TagBase(tag_id, name, memRes), values{ values, memRes } {
		}
		//Copy constructor
		ArrayType_Tag(const ArrayType_Tag& copyFrom)
			: NBT_TagBase(tag_id, { copyFrom.name, copyFrom.name.get_allocator() }, copyFrom.name.get_allocator().resource())
			, values{ copyFrom.values, copyFrom.values.get_allocator() }{
		}
		//Move constructor
		ArrayType_Tag(ArrayType_Tag&& moveFrom) noexcept
			: NBT_TagBase(tag_id, { std::move(moveFrom.name), moveFrom.name.get_allocator() }, moveFrom.name.get_allocator().resource())
			, values{ std::move(moveFrom.values) } {
		}

		static ArrayType_Tag fromRawData(const decltype(name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes) {
			if (maxReadLength < sizeof(int32_t))
				throw std::out_of_range("Data ran out while reading length of " + TagIDToString(tag_id) + ": " + std::string{ name });

			int32_t count = copyAndFlipBytes<int32_t>(dataPtr);
			maxReadLength -= sizeof(count);
			dataPtr += sizeof(count);

			if (maxReadLength < count * sizeof(valueType))
				throw std::out_of_range("Data ran out while reading values of " + TagIDToString(tag_id) + ": " + std::string{ name });

			decltype(values) valArray{ (size_t)count, {}, memRes };
			for (valueType& val : valArray) {
				val = copyAndFlipBytes<valueType>(dataPtr);
				dataPtr += sizeof(valueType);
			}

			out_bytesRead = sizeof(count) + count * sizeof(valueType);
			return ArrayType_Tag<valueType, tag_id>(name, valArray, memRes);
		}

		void addTagToBinaryStream(BinaryStream& bstream) const override {
			const int32_t flippedLength{ byteswap(static_cast<int32_t>(values.size())) };
			bstream.pushbackData(&flippedLength, sizeof(flippedLength));
			
			for (valueType val : values) {
				valueType temp{ byteswap(val) };
				bstream.pushbackData(&temp, sizeof(valueType));
			}
		}

		void addToStringStream(std::stringstream& ss, uint8_t tabDepth) const {
			addTabsToStringStream(ss, tabDepth);
			ss << TagIDToString(id) << ": " << name << " = {";
			for (size_t i = 0; i < values.size() - 1; ++i) {
				if (sizeof(valueType) == sizeof(int8_t))
					ss << int32_t(values[i]);
				else
					ss << values[i];
				ss << ", ";
			}
			if (sizeof(valueType) == sizeof(int8_t))
				ss << int32_t(values.back());
			else
				ss << values.back();
			ss << '}';
		}
	};

	using ByteArray_Tag = ArrayType_Tag<int8_t, TagID::Byte_Array>;
	using IntArray_Tag = ArrayType_Tag<int32_t, TagID::Int_Array>;
	using LongArray_Tag = ArrayType_Tag<int64_t, TagID::Long_Array>;
	
	struct String_Tag : public NBT_TagBase {
		decltype(name) value;
		String_Tag(const decltype(name)& name, decltype(value) value, std::pmr::memory_resource* memRes) noexcept
			: NBT_TagBase(TagID::String, name, memRes), value{ value, memRes } {
		}
		//Copy constructor
		String_Tag(const String_Tag& copyFrom)
			: NBT_TagBase(TagID::String, { copyFrom.name, copyFrom.name.get_allocator() }, copyFrom.name.get_allocator().resource())
			, value{ copyFrom.value, copyFrom.value.get_allocator() }{
		}
		//Move constructor
		String_Tag(String_Tag&& moveFrom) noexcept
			: NBT_TagBase(TagID::String, { std::move(moveFrom.name), moveFrom.name.get_allocator() }, moveFrom.name.get_allocator().resource())
			, value{ std::move(moveFrom.value), moveFrom.value.get_allocator()} {
		}

		static String_Tag inline fromRawData(const decltype(name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes) {
			if (maxReadLength < sizeof(int16_t))
				throw std::out_of_range("Data ran out while reading length of TAG_String: " + std::string{ name });

			int16_t count = copyAndFlipBytes<int16_t>(dataPtr);
			maxReadLength -= sizeof(count);
			dataPtr += sizeof(count);

			if (maxReadLength < count * sizeof(char))
				throw std::out_of_range("Data ran out while reading characters of TAG_String: " + std::string{ name });

			out_bytesRead = sizeof(int16_t) + count;
			return String_Tag(name, decltype(value)(reinterpret_cast<char*>(dataPtr), count, memRes), memRes);
		}

		void addTagToBinaryStream(BinaryStream& bstream) const override {
			const int16_t flippedStringLength{ byteswap(static_cast<int16_t>(value.size())) };
			bstream.pushbackData(&flippedStringLength, sizeof(flippedStringLength));

			bstream.pushbackData(value.data(), value.size());
		}

		void addToStringStream(std::stringstream& ss, uint8_t tabDepth) const {
			addTabsToStringStream(ss, tabDepth);
			ss << TagIDToString(id) << ": " << name << " = " << value;
		}
	};

	NBT_TagBase* constructNewTag(TagID id, const decltype(NBT_TagBase::name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes);
	
	void deallocTag(TagID id, NBT_TagBase* ptr, std::pmr::memory_resource* memRes);

	struct List_Tag : public NBT_TagBase {
		TagID listType;
		std::pmr::vector<NBT_TagBase*> values;
		List_Tag(const decltype(name)& name, TagID listType, decltype(values) values, std::pmr::memory_resource* memRes)
			: NBT_TagBase(TagID::List, name, memRes), listType{ listType }, values{ values, memRes} {
		}
		//Copy constructor
		List_Tag(const List_Tag& copyFrom) = delete;
		List_Tag(const List_Tag& copyFrom, std::pmr::memory_resource* memRes)
			: NBT_TagBase(TagID::List, { copyFrom.name, copyFrom.name.get_allocator() }, copyFrom.name.get_allocator().resource())
			, values{ copyFrom.values, copyFrom.values.get_allocator() }, listType{ copyFrom.listType }{

			for (size_t i = 0u; i < values.size(); ++i) {
				values[i] = copyTag(values[i], memRes);
			}
		}
		//Move constructor
		List_Tag(List_Tag&& moveFrom) noexcept
			: NBT_TagBase(TagID::List, { std::move(moveFrom.name), moveFrom.name.get_allocator() }, moveFrom.name.get_allocator().resource())
			, values{ std::move(moveFrom.values) }, listType{ moveFrom.listType } {
		}

		~List_Tag() {
			for (auto& v : values)
				deallocTag(listType, v, values.get_allocator().resource());
		}

		static List_Tag fromRawData(const decltype(name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes);

		void addTagToBinaryStream(BinaryStream& bstream) const override;
		void addToStringStream(std::stringstream& ss, uint8_t tabDepth) const;

	private:
		template<typename valueType> requires std::derived_from<valueType, NBT_TagBase>
		void addListToBinaryStream(BinaryStream& bstream) const {
			for (const auto& ptr : values) {
				reinterpret_cast<valueType*>(ptr)->addTagToBinaryStream(bstream);
			}
		}
	};

	struct Compound_Tag : public NBT_TagBase { //TODO maybe use a different hashtable and function.
		std::pmr::vector<NBT_TagBase*> values;
		std::pmr::unordered_map<std::pmr::string, size_t> indexMap; //string key, size_t value is the index in the vector.
		
		Compound_Tag(const decltype(name)& name, decltype(values) values, std::pmr::memory_resource* memRes)
			: NBT_TagBase(TagID::Compound, name, memRes), values{ values, memRes }, indexMap{ memRes } {

			for (size_t i = 0u; i < values.size(); ++i) {
				indexMap[values[i]->name] = i;
			}
		}
		//Copy constructor
		Compound_Tag(const Compound_Tag& copyFrom) = delete;
		Compound_Tag(const Compound_Tag& copyFrom, std::pmr::memory_resource* memRes)
			: NBT_TagBase(TagID::Compound, { copyFrom.name, memRes }, memRes)
			, values{ copyFrom.values, memRes }, indexMap{ copyFrom.indexMap, memRes }{

			for (size_t i = 0u; i < values.size(); ++i) {
				values[i] = copyTag(values[i], memRes);
			}
		}
		//Move constructor
		Compound_Tag(Compound_Tag&& moveFrom) noexcept
			: NBT_TagBase(TagID::Compound, { std::move(moveFrom.name), moveFrom.name.get_allocator()}, moveFrom.name.get_allocator().resource())
			, values{ std::move(moveFrom.values) }, indexMap{ std::move(moveFrom.indexMap) } {
		}

		~Compound_Tag() {
			for (auto& v : values) {
				deallocTag(v->id, v, values.get_allocator().resource());
			}
		}

		void inline addTag(NBT_TagBase* tagPtr) {
			values.push_back(tagPtr);
			indexMap[tagPtr->name] = values.size() - 1u;
		}

		static Compound_Tag fromRawData(const decltype(name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes);

		void addTagToBinaryStream(BinaryStream& bstream) const override;
		void addToStringStream(std::stringstream& ss, uint8_t tabDepth) const;

	private:
		template<typename valueType> requires std::derived_from<valueType, NBT_TagBase>
		void addFullTagToBinaryStream(NBT_TagBase* tagPtr, BinaryStream& bstream) const {
			valueType* ptr{ reinterpret_cast<valueType*>(tagPtr) };
			ptr->addTagHeaderToBinaryStream(bstream);
			ptr->addTagToBinaryStream(bstream);
		}
	};

	Compound_Tag parseNBT(void* dataPtr, size_t dataSize, std::pmr::memory_resource* memRes);

	std::vector<byte> buildBinaryNBTFile(const Compound_Tag* root);

}
