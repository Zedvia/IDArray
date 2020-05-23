
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>


// DO NOT CHANGE THE UNDERLYING TYPE!
// our MeshID has 32 bits. you can use those bits for whatever purpose you see fit.
// this is the only thing our user sees.
typedef uint32_t MeshID;


// DO NOT CHANGE!
// you can assume that we never store more than 256 entries.
static const unsigned int MAX_MESH_COUNT = 256;


// DO NOT CHANGE!
// your implementation needs to work with arbitrary types, whose layout we cannot change.
// you can assume that all types you're going to deal with are PODs, like the Mesh here.
struct Mesh
{
	// the mesh usually has several members (vertex buffer, index buffer, etc.).
	// for our exercise we only use a dummy member. this is only used to check whether the implementation works.
	int dummy;
};


// in our exercise, the RenderWorld only holds meshes, nothing more.
// the RenderWorld implementation makes sure that after *each* operation its internal data structure has no holes,
// and contains Mesh instances which are stored contiguously in memory.
class RenderWorld
{
public:
	RenderWorld(void)
		: m_meshCount(0)
	{
	}


	// Needs to be O(1)
	MeshID AddMesh(void)
	{
		// TODO: add your implementation here.
		// thoroughly comment *what* you do, and *why* you do it (in german or english).
		Mesh mesh{};
		m_meshes[m_meshCount] = mesh;
		

		sparse_id_mask mask{};
		mask.index = m_meshCount;
		mask.generation = 0; // todo vorher holen und dann grüßer oder so
		
		m_sparse_array.emplace_back(mask);
		
		MeshID ID = ((m_sparse_array.size() - 1) << generationSize) + mask.generation; // todo change m_meshcount

		m_denseToSparse[m_meshCount] = m_sparse_array.size() - 1;
		
		// add to freelist, add to dense array, increase m_meshCount
		// MeshID should save index in freelist and generation?
		// freelist needs to store IN PLACE, so no vector, list etc allowed
		m_meshCount++;

		return ID;
	}


	// Needs to be O(1)
	void RemoveMesh(MeshID id)
	{
		// TODO: add your implementation here.
		// thoroughly comment *what* you do, and *why* you do it (in german or english).


		sparse_id_mask meshID{};
		meshID.index = (id & index_mask) >> generationSize;
		meshID.generation = (id & generation_mask);
		// checken ob generation gleich
		sparse_id_mask entry = m_sparse_array[meshID.index];

		if (entry.generation == meshID.generation)
		{
			// mesh mit letzten im dense swappen
			std::swap(m_meshes[entry.index], m_meshes[m_meshCount-1]);
			// in sparse für geswappten mesh index aktualisieren
			m_sparse_array[m_denseToSparse[m_meshCount-1]].index = entry.index;
			// in sparse generation erhöhen
			m_sparse_array[m_denseToSparse[meshID.index]].generation++;
			// denseToSparse aktualisieren
			std::swap(m_denseToSparse[m_meshCount-1], m_denseToSparse[meshID.index]);
			// m_meshCount reduzieren
			m_meshCount--;
		}
	}


	// Needs to be O(1)
	Mesh* Lookup(MeshID id)
	{
		// TODO: add your implementation here.
		// thoroughly comment *what* you do, and *why* you do it (in german or english).
		
		
		sparse_id_mask meshID{};
		meshID.index = (id & index_mask) >> generationSize;
		meshID.generation = (id & generation_mask);
		
		//uint8_t index = (id & index_mask) >> generationSize;
		//uint32_t generation = (id & generation_mask);

		sparse_id_mask entry = m_sparse_array[meshID.index];

		if(entry.generation == meshID.generation)
		{
			return &m_meshes[entry.index];
		}
		
		return nullptr;
	}


	// DO NOT CHANGE!
	// the implementation of this method needs to stay as it is.
	// you need to correctly implement all other methods to ensure that:
	// a) m_meshCount is up-to-date
	// b) m_meshes stores instances of Mesh contiguously, without any holes
	// c) external MeshIDs still refer to the correct instances somehow
	void Iterate(void)
	{
		for (unsigned int i = 0; i < m_meshCount; ++i)
		{
			printf("Mesh instance %d: dummy = %d\n", i, m_meshes[i].dummy);
		}
	}


private:
	static const unsigned int index_mask = 0xFF000000;
	static const unsigned int generation_mask = 0x00FFFFFF;
	static const unsigned short indexSize = 8;
	static const unsigned short generationSize = 24;
	
	struct sparse_id_mask
	{
		uint32_t index : indexSize,
		    generation : generationSize;
	};

	std::vector<sparse_id_mask> m_sparse_array;
	uint32_t m_denseToSparse[MAX_MESH_COUNT];
	
	// DO NOT CHANGE!
	// these two members are here to stay. see comments regarding Iterate().
	Mesh m_meshes[MAX_MESH_COUNT];
	unsigned int m_meshCount;
};



int main(void)
{
	RenderWorld rw;

	// add 3 meshes to the world. we only ever refer to them by their ID, the RenderWorld has complete ownership
	// over the individual Mesh instances.
	MeshID meshID0 = rw.AddMesh();
	MeshID meshID1 = rw.AddMesh();
	MeshID meshID2 = rw.AddMesh();

	// lookup the meshes, and fill them with data.
	{
		Mesh* mesh0 = rw.Lookup(meshID0);
		mesh0->dummy = 0;
		Mesh* mesh1 = rw.Lookup(meshID1);
		mesh1->dummy = 1;
		Mesh* mesh2 = rw.Lookup(meshID2);
		mesh2->dummy = 2;
	}

	// by now, the world contains 3 meshes, filled with dummy data 0, 1 and 2.
	// in memory, the 3 meshes should be contiguous in memory:
	// [Mesh][Mesh][Mesh]
	rw.Iterate();

	// we now remove the second mesh (referenced by meshID1), which creates a hole in the world's data structure:
	// [Mesh][Empty][Mesh]
	// the world should internally update its data structure(s), so that the other two remaining meshes are stored contiguously in memory.
	rw.RemoveMesh(meshID1);

	// iteration must still work, because the instances are contiguous in memory.
	rw.Iterate();

	// even though the render world might have copied/changed some data structures, the two remaining meshes must still
	// refer to the correct object. this is verified by checking their dummy members.
	assert(rw.Lookup(meshID0)->dummy == 0);
	assert(rw.Lookup(meshID2)->dummy == 2);

	// the mesh referenced by meshID1 has been removed above, yet we intentionally try to access it.
	// the implementation should give an error, and return a nullptr in that case.
	Mesh* mesh1 = rw.Lookup(meshID1);
	assert(mesh1 == nullptr);

	return 0;
}
