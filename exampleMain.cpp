#include <iostream>
#include <fstream>
#include <vector>
#include "NBT_Lib.h"


std::vector<char> loadBinaryFile(std::string filepath) {
	std::ifstream filestream(filepath, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);

	if (!filestream.is_open() || filestream.fail())
		throw std::runtime_error(std::string("File could not be opened: ") + filepath);

	size_t fileSize = filestream.tellg();
	std::vector<char> data(fileSize);

	filestream.seekg(0, std::ios_base::beg);
	filestream.read(data.data(), fileSize);

	filestream.close();

	return data;
}

NBT_Lib::Compound_Tag Example_Compose_NBT( std::pmr::memory_resource* memRes) {
	using namespace NBT_Lib;
	Compound_Tag root("", {}, memRes);

	root.addTag(new(allocateMemory<String_Tag>(memRes)) String_Tag("Example_String", "Example", memRes));

	LongArray_Tag* longArr{ new(allocateMemory<LongArray_Tag>(memRes)) LongArray_Tag("Longs", {64u, 128u, 256u, 512u, 1028u, 2056u, 4112u, 8224u, 16448u}, memRes) };
	root.addTag(longArr);

	List_Tag* floatList{ new(allocateMemory<List_Tag>(memRes)) List_Tag("Float_List", TagID::Float, {}, memRes)};
	for (size_t i = 0u; i < 8u; ++i) {
		floatList->values.push_back(new(allocateMemory<Float_Tag>(memRes)) Float_Tag("", float(i) * .3f, memRes));
	}
	root.addTag(floatList);

	List_Tag* compList{ new(allocateMemory<List_Tag>(memRes)) List_Tag("Compound_List", TagID::Compound, {}, memRes) };
	root.addTag(compList);

	Compound_Tag* comp{ new(allocateMemory<Compound_Tag>(memRes)) Compound_Tag("", {}, memRes) };
	compList->values.push_back(comp);
	comp->addTag(copyTag(longArr, memRes));
	comp->addTag(copyTag(floatList, memRes));

	compList->values.push_back(copyTag(comp, memRes));

	comp->addTag(copyTag(compList->values.back(), memRes));

	return root;
}

NBT_Lib::Compound_Tag Example_Read_NBT_From_File(std::pmr::memory_resource* memRes) {
	std::vector<char> data = loadBinaryFile("testdata/level_TestFile.rawNBT");
	auto root{ NBT_Lib::parseNBT((std::byte*)data.data(), data.size(), memRes) };
	return root;
}

std::vector<std::byte> Example_Encode_NBT_As_Binary(std::pmr::memory_resource* memRes) {
	auto root{ Example_Compose_NBT(memRes) };
	auto data{ NBT_Lib::buildBinaryNBTFile(&root) };
	return data;
}
/*
int main() {

	std::pmr::monotonic_buffer_resource backBuffer;
	std::pmr::unsynchronized_pool_resource memRes(&backBuffer);

	
	//auto root{ Example_Read_NBT_From_File(&memRes) };

	//auto root2{ Example_Compose_NBT(&memRes) };

	//auto bin{ Example_Encode_NBT_As_Binary(&memRes)};
}
*/