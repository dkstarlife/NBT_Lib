#include "NBT_Lib.h"
namespace NBT_Lib {
	Compound_Tag parseNBT(void* dataPtr, size_t dataSize, std::pmr::memory_resource* memRes) {
		size_t dataRead{ 0u };
		byte* data{ reinterpret_cast<byte*>(dataPtr) };

		if (data[0] != static_cast<byte>(TagID::Compound))
			throw std::runtime_error("Root tag must be TAG_Compound, but it was " + TagIDToString(static_cast<TagID>(data[0])));

		int16_t namelength;
		memcpy(&namelength, data+sizeof(int8_t), sizeof(int16_t));
		data += sizeof(int8_t) + sizeof(int16_t) + size_t(namelength);

		return Compound_Tag::fromRawData(std::pmr::string("", memRes), data, dataSize, dataRead, memRes);
	}

	std::vector<byte> buildBinaryNBTFile(const Compound_Tag* root) {
		BinaryStream bstream;
		root->addTagHeaderToBinaryStream(bstream);
		root->addTagToBinaryStream(bstream);

		return bstream.getData();
	}

	NBT_TagBase* constructNewTag(TagID id, const decltype(NBT_TagBase::name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes) {
		switch (id) {
			using enum TagID;
		case End: {
			auto tagPtr = allocateMemory<End_Tag>(memRes);
			new(tagPtr) End_Tag{ memRes };
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Byte: {
			auto tagPtr = allocateMemory<Byte_Tag>(memRes);
			new(tagPtr) Byte_Tag(Byte_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Short: {
			auto tagPtr = allocateMemory<Short_Tag>(memRes);
			new(tagPtr) Short_Tag(Short_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Int: {
			auto tagPtr = allocateMemory<Int_Tag>(memRes);
			new(tagPtr) Int_Tag(Int_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Long: {
			auto tagPtr = allocateMemory<Long_Tag>(memRes);
			new(tagPtr) Long_Tag(Long_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Float: {
			auto tagPtr = allocateMemory<Float_Tag>(memRes);
			new(tagPtr) Float_Tag(Float_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Double: {
			auto tagPtr = allocateMemory<Double_Tag>(memRes);
			new(tagPtr) Double_Tag(Double_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Byte_Array: {
			auto tagPtr = allocateMemory<ByteArray_Tag>(memRes);
			new(tagPtr) ByteArray_Tag(ByteArray_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case String: {
			auto tagPtr = allocateMemory<String_Tag>(memRes);
			new(tagPtr) String_Tag(String_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case List: {
			auto tagPtr = allocateMemory<List_Tag>(memRes);
			new(tagPtr) List_Tag(List_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Compound: {
			auto tagPtr = allocateMemory<Compound_Tag>(memRes);
			new(tagPtr) Compound_Tag(Compound_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Int_Array: {
			auto tagPtr = allocateMemory<IntArray_Tag>(memRes);
			new(tagPtr) IntArray_Tag(IntArray_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		case Long_Array: {
			auto tagPtr = allocateMemory<LongArray_Tag>(memRes);
			new(tagPtr) LongArray_Tag(LongArray_Tag::fromRawData(name, dataPtr, maxReadLength, out_bytesRead, memRes));
			return static_cast<NBT_TagBase*>(tagPtr);
		}
		default:
			throw std::runtime_error("Attempted to construct an NBT tag from an unknown tag id.");
		}
		return nullptr;
	}

	template<typename tagType>
	inline NBT_TagBase* copySimple(NBT_TagBase* tagPtr, std::pmr::memory_resource* memRes) {
		tagType* ptr{ allocateMemory<tagType>(memRes) };
		new(ptr) tagType(*(reinterpret_cast<tagType*>(tagPtr)));
		return reinterpret_cast<NBT_TagBase*>(ptr);
	}

	NBT_TagBase* copyTag(NBT_TagBase* tagPtr, std::pmr::memory_resource* memRes) {
		switch (tagPtr->id) {
			using enum TagID;
		case End:
			return copySimple<End_Tag>(tagPtr, memRes);
		case Byte:
			return copySimple<Byte_Tag>(tagPtr, memRes);
		case Short:
			return copySimple<Short_Tag>(tagPtr, memRes);
		case Int:
			return copySimple<Int_Tag>(tagPtr, memRes);
		case Long:
			return copySimple<Long_Tag>(tagPtr, memRes);
		case Float:
			return copySimple<Float_Tag>(tagPtr, memRes);
		case Double:
			return copySimple<Double_Tag>(tagPtr, memRes);
		case Byte_Array:
			return copySimple<ByteArray_Tag>(tagPtr, memRes);
		case String:
			return copySimple<String_Tag>(tagPtr, memRes);
		case List: {
			auto newPtr = allocateMemory<List_Tag>(memRes);
			List_Tag* other{ static_cast<List_Tag*>(tagPtr) };
			new(newPtr) List_Tag(*other, memRes);
			return static_cast<NBT_TagBase*>(newPtr);
		}
		case Compound: {
			auto newPtr = allocateMemory<Compound_Tag>(memRes);
			Compound_Tag* other{ reinterpret_cast<Compound_Tag*>(tagPtr) };
			new(newPtr) Compound_Tag(*other, memRes);
			return reinterpret_cast<NBT_TagBase*>(newPtr);
		}
		case Int_Array:
			return copySimple<IntArray_Tag>(tagPtr, memRes);
		case Long_Array:
			return copySimple<LongArray_Tag>(tagPtr, memRes);
		default:
			throw std::runtime_error("Attempted to construct an NBT tag from an unknown tag id.");
		}
		return nullptr;
	}

	void deallocTag(TagID id, NBT_TagBase* ptr, std::pmr::memory_resource* memRes) {
		switch (id) {
			using enum TagID;
		case End:
			deallocateMemory<End_Tag>(ptr, memRes);
			break;
		case Byte:
			deallocateMemory<Byte_Tag>(ptr, memRes);
			break;
		case Short:
			deallocateMemory<Short_Tag>(ptr, memRes);
			break;
		case Int:
			deallocateMemory<Int_Tag>(ptr, memRes);
			break;
		case Long:
			deallocateMemory<Long_Tag>(ptr, memRes);
			break;
		case Float:
			deallocateMemory<Float_Tag>(ptr, memRes);
			break;
		case Double:
			deallocateMemory<Double_Tag>(ptr, memRes);
			break;
		case Byte_Array:
			deallocateMemory<ByteArray_Tag>(ptr, memRes);
			break;
		case String:
			deallocateMemory<String_Tag>(ptr, memRes);
			break;
		case List:
			deallocateMemory<List_Tag>(ptr, memRes);
			break;
		case Compound:
			deallocateMemory<Compound_Tag>(ptr, memRes);
			break;
		case Int_Array:
			deallocateMemory<IntArray_Tag>(ptr, memRes);
			break;
		case Long_Array:
			deallocateMemory<LongArray_Tag>(ptr, memRes);
			break;

		}
	}

	List_Tag List_Tag::fromRawData(const decltype(name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes) {
		if (maxReadLength < sizeof(int8_t))
			throw std::out_of_range("Data ran out while reading listType of TAG_List: " + std::string{ name });

		TagID listType{ static_cast<TagID>(dataPtr[0]) };
		++dataPtr;
		--maxReadLength;

		if (maxReadLength < sizeof(int32_t))
			throw std::out_of_range("Data ran out while reading length of TAG_List: " + std::string{ name });

		int32_t count = copyAndFlipBytes<int32_t>(dataPtr);
		maxReadLength -= sizeof(count);
		dataPtr += sizeof(count);

		out_bytesRead = sizeof(int8_t) + sizeof(int32_t);

		decltype(values) tempVals{ (size_t)count, nullptr, memRes };
		for (auto& ptr : tempVals) {
			size_t bytesRead{ 0u };
			ptr = constructNewTag(listType, std::pmr::string(memRes), dataPtr, maxReadLength, bytesRead, memRes);
			out_bytesRead += bytesRead;
			dataPtr += bytesRead;
			maxReadLength -= bytesRead;
		}

		return List_Tag(name, listType, tempVals, memRes);
	}

	void List_Tag::addTagToBinaryStream(BinaryStream& bstream) const {
		byte listHeader[1u + sizeof(int32_t)];
		listHeader[0] = static_cast<byte>(static_cast<int8_t>(listType));
		const int32_t flippedListLength{ byteswap(static_cast<int32_t>(values.size())) };
		memcpy(listHeader + 1u, &flippedListLength, sizeof(flippedListLength));
		bstream.pushbackData(listHeader, sizeof(listHeader));

		switch (listType) {
			using enum TagID;
		case End:
			//addListToBinaryStream<End_Tag>(bstream);
			break;
		case Byte:
			addListToBinaryStream<Byte_Tag>(bstream);
			break;
		case Short:
			addListToBinaryStream<Short_Tag>(bstream);
			break;
		case Int:
			addListToBinaryStream<Int_Tag>(bstream);
			break;
		case Long:
			addListToBinaryStream<Long_Tag>(bstream);
			break;
		case Float:
			addListToBinaryStream<Float_Tag>(bstream);
			break;
		case Double:
			addListToBinaryStream<Double_Tag>(bstream);
			break;
		case Byte_Array:
			addListToBinaryStream<ByteArray_Tag>(bstream);
			break;
		case String:
			addListToBinaryStream<String_Tag>(bstream);
			break;
		case List:
			addListToBinaryStream<List_Tag>(bstream);
			break;
		case Compound:
			addListToBinaryStream<Compound_Tag>(bstream);
			break;
		case Int_Array:
			addListToBinaryStream<IntArray_Tag>(bstream);
			break;
		case Long_Array:
			addListToBinaryStream<LongArray_Tag>(bstream);
			break;
		}
	}

	Compound_Tag Compound_Tag::fromRawData(const decltype(name)& name, byte* dataPtr, size_t maxReadLength, size_t& out_bytesRead, std::pmr::memory_resource* memRes) {
		out_bytesRead = 0u;

		decltype(values) tempValues{ memRes };

		while (maxReadLength > 0u) {
			if (maxReadLength < sizeof(int8_t))
				throw std::out_of_range("Data ran out while reading tag type of element in TAG_Compound: " + std::string{ name });

			TagID elemType{ static_cast<TagID>(dataPtr[0]) };
			if (elemType > TagID::Long_Array)
				throw std::runtime_error("Invalid tag id encountered in TAG_Compound: " + std::string(name));

			++dataPtr;
			--maxReadLength;

			if (elemType == TagID::End) { //End tag signifies the end of the compound tag.
				//size_t elemBytesRead{ 0u };
				//NBT_TagBase* elemPtr = constructNewTag(elemType, std::pmr::string(memRes), dataPtr, maxReadLength, elemBytesRead, memRes);
				//tempValues.push_back(elemPtr);
				break;
			}

			if (maxReadLength < sizeof(int16_t))
				throw std::out_of_range("Data ran out while reading name of element in TAG_Compound: " + std::string{ name });

			size_t nameLength{ static_cast<size_t>(copyAndFlipBytes<int16_t>(dataPtr)) };
			dataPtr += sizeof(int16_t);
			maxReadLength -= sizeof(int16_t);

			decltype(name) elemName{ reinterpret_cast<char*>(dataPtr), nameLength, memRes };
			dataPtr += nameLength;
			maxReadLength -= nameLength;

			size_t elemBytesRead{ 0u };
			NBT_TagBase* elemPtr = constructNewTag(elemType, elemName, dataPtr, maxReadLength, elemBytesRead, memRes);

			tempValues.push_back(elemPtr);

			out_bytesRead += sizeof(int8_t) + sizeof(int16_t) + nameLength + elemBytesRead;
			dataPtr += elemBytesRead;
			maxReadLength -= elemBytesRead;
		}

		return Compound_Tag(name, tempValues, memRes);
	}

	void Compound_Tag::addTagToBinaryStream(BinaryStream& bstream) const {

		for (NBT_TagBase* ptr : values) {

			switch (ptr->id) {
				using enum TagID;
			case End:
				//reinterpret_cast<End_Tag*>(ptr)->addTagHeaderToBinaryStream(bstream);
				break;
			case Byte:
				addFullTagToBinaryStream<Byte_Tag>(ptr, bstream);
				break;
			case Short:
				addFullTagToBinaryStream<Short_Tag>(ptr, bstream);
				break;
			case Int:
				addFullTagToBinaryStream<Int_Tag>(ptr, bstream);
				break;
			case Long:
				addFullTagToBinaryStream<Long_Tag>(ptr, bstream);
				break;
			case Float:
				addFullTagToBinaryStream<Float_Tag>(ptr, bstream);
				break;
			case Double:
				addFullTagToBinaryStream<Double_Tag>(ptr, bstream);
				break;
			case Byte_Array:
				addFullTagToBinaryStream<ByteArray_Tag>(ptr, bstream);
				break;
			case String:
				addFullTagToBinaryStream<String_Tag>(ptr, bstream);
				break;
			case List:
				addFullTagToBinaryStream<List_Tag>(ptr, bstream);
				break;
			case Compound:
				addFullTagToBinaryStream<Compound_Tag>(ptr, bstream);
				break;
			case Int_Array:
				addFullTagToBinaryStream<IntArray_Tag>(ptr, bstream);
				break;
			case Long_Array:
				addFullTagToBinaryStream<LongArray_Tag>(ptr, bstream);
				break;
			}
		}

		//Add an end tag to signify the end of the compound.
		//End_Tag endtag{};
		//endtag.addTagHeaderToBinaryStream(bstream);
		byte endtag{ static_cast<byte>(TagID::End) };
		bstream.pushbackData(&endtag, sizeof(endtag));
	}

	void addToStringStreamHelper(std::stringstream& ss, uint8_t tabDepth, NBT_TagBase* tag) {
		switch (tag->id) {
			using enum TagID;
		case End:
			reinterpret_cast<End_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Byte:
			reinterpret_cast<Byte_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Short:
			reinterpret_cast<Short_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Int:
			reinterpret_cast<Int_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Long:
			reinterpret_cast<Long_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Float:
			reinterpret_cast<Float_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Double:
			reinterpret_cast<Double_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Byte_Array:
			reinterpret_cast<ByteArray_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case String:
			reinterpret_cast<String_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case List:
			reinterpret_cast<List_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Compound:
			reinterpret_cast<Compound_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Int_Array:
			reinterpret_cast<IntArray_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		case Long_Array:
			reinterpret_cast<LongArray_Tag*>(tag)->addToStringStream(ss, tabDepth);
			break;
		}
	}

	void List_Tag::addToStringStream(std::stringstream& ss, uint8_t tabDepth) const {
		addTabsToStringStream(ss, tabDepth);
		ss << TagIDToString(id) << ": " << name << " = {\n";
		for (const auto& val : values) {
			addToStringStreamHelper(ss, tabDepth + 1u, val);
			ss << "\n";
		}
		addTabsToStringStream(ss, tabDepth);
		ss << '}';
	}
	void Compound_Tag::addToStringStream(std::stringstream& ss, uint8_t tabDepth) const {
		addTabsToStringStream(ss, tabDepth);
		ss << TagIDToString(id) << ": " << name << " = {\n";
		for (const auto& val : values) {
			addToStringStreamHelper(ss, tabDepth + 1u, val);
			ss << "\n";
		}
		addTabsToStringStream(ss, tabDepth);
		ss << '}';
	}
}