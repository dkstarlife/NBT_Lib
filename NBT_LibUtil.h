#pragma once
#include <bit>
namespace NBT_Lib {
	using std::byte;

	template<typename valueType>
	[[nodiscard]]
	inline constexpr valueType byteswap(const valueType val) {
		valueType flipped;
		for (size_t i = 0u; i < sizeof(valueType); ++i) {
			memcpy((&flipped) + i, (&val) + (sizeof(valueType) - 1u - i), 1u);
		}
		return flipped;
	}
#ifdef __cpp_lib_byteswap
	template<>
	[[nodiscard]]
	inline constexpr float byteswap(const float val) {
		return std::bit_cast<float>(std::byteswap(std::bit_cast<uint32_t>(val)));
	}
	template<>
	[[nodiscard]]
	inline constexpr double byteswap(const double val) {
		return std::bit_cast<double>(std::byteswap(std::bit_cast<uint64_t>(val)));
	}
	template<std::integral valueType>
	[[nodiscard]]
	inline constexpr valueType byteswap(const valueType val) {
		return std::byteswap(val);
	}
#endif

	template<typename T>
	[[nodiscard]]
	inline T copyAndFlipBytes(byte* src) {
		T value;
		memcpy(&value, src, sizeof(value));
		return byteswap(value);
	}

	template<typename objectType>
	[[nodiscard]]
	inline objectType* allocateMemory(std::pmr::memory_resource* memRes) {
		return static_cast<objectType*>(memRes->allocate(sizeof(objectType), alignof(objectType)));
	}
	template<typename objectType>
	inline void deallocateMemory(void* ptr, std::pmr::memory_resource* memRes) {
		static_cast<objectType*>(ptr)->~objectType();
		memRes->deallocate(static_cast<objectType*>(ptr), sizeof(objectType), alignof(objectType));
	}

	class BinaryStream {
		const size_t chunkAllocSize{ 1u << 10u };
		std::vector<byte*> chunks; //chunk data ptr.
		size_t cursor = 0u; //number of bytes written to the active chunk.
	public:
		BinaryStream() {
			chunks.push_back(new byte[chunkAllocSize]);
		}
		~BinaryStream() {
			for (auto& elem : chunks)
				delete[] elem;
		}

		//Add some bytes to the end of the buffer.
		void pushbackData(const void* data, size_t size) { //TODO test
			byte* dataPtr{ (byte*)data };
			while (size != 0u) {
				if (cursor + size < chunkAllocSize) { //if everthing fits in the active chunk.
					memcpy(chunks.back() + cursor, dataPtr, size);
					cursor += size;
					return;
				}
				else {
					//copy what ever is able to fit in the currently active chunk.
					const size_t copySize{ (chunkAllocSize - cursor) };
					memcpy(chunks.back() + cursor, dataPtr, copySize);
					size -= copySize;
					dataPtr += copySize;
					//make a new chunk and copy the rest.
					cursor = 0u;
					chunks.push_back(new byte[chunkAllocSize]);
				}
			}
		}

		//Packs all the data into a single buffer and returns it.
		[[nodiscard]]
		std::vector<byte> getData() {
			const size_t totalSize{ (chunks.size() - 1u) * chunkAllocSize + cursor };
			std::vector<byte> data(totalSize);
			byte* dataPtr{ data.data() };
			for (size_t i = 0u; i < (chunks.size() - 1); ++i) {
				memcpy(dataPtr, chunks[i], chunkAllocSize);
				dataPtr += chunkAllocSize;
			}
			//copy trailing data contained in the last chunk.
			memcpy(dataPtr, chunks.back(), cursor);

			return data;
		}
	};

}
