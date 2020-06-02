
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <queue>
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

//Benutzte struct und union um sie unabhängig von der Klasse zu speichern
struct id_struct
{
	uint32_t generation : 24,
			 index      :  8;
};

//für einfachen Zugriff auf entweder die 2 einzelnen Werte oder die generelle MeshID
union id_converter
{
	MeshID ID;
	id_struct mask;
};
/////////

// in our exercise, the RenderWorld only holds meshes, nothing more.
// the RenderWorld implementation makes sure that after *each* operation its internal data structure has no holes,
// and contains Mesh instances which are stored contiguously in memory.
class RenderWorld
{
public:
	RenderWorld(void)
		: m_meshCount(0)
	{
		//Hier wird ein uint16_t benutzt um die for Bedingung auch erfüllen zu können, mit einem uint8_t wären wir hier in einem unendlichen loop
		for (uint16_t i = 0; i < MAX_MESH_COUNT; ++i)
		{
			m_open_ids.emplace_back(i);
		}
	}


	// Needs to be O(1)
	MeshID AddMesh(void)
	{
		// TODO: add your implementation here.
		// thoroughly comment *what* you do, and *why* you do it (in german or english).

		//Mesh wird an die richtige Stelle im m_meshes (dense) array gespeichert
		m_meshes[m_meshCount] = Mesh();

		//nächste freie ID im m_sparse_array wird geholt
		uint8_t free_id = m_open_ids.front();
		m_open_ids.pop_front();

		//Daten für sparse_array werden geschrieben: position im dense array und generation
		id_struct mask{};
		mask.index = m_meshCount;
		mask.generation = m_sparse_array[free_id].generation;
		
		m_sparse_array[free_id] = mask;

		//dense to sparse array wird am index des meshes, welches das mesh auch im dense array hat,
		//mit der id für den Eintrag im sparse array befüllt um Zugriff in beide Richtungen zu ermöglichen
		m_denseToSparse[m_meshCount] = free_id;
		
		// m_meshCount erhöhen damit neue Meshes an die richtige Stelle geaddet werden
		m_meshCount++;

		//MeshID wird mit der Generation und dem Index im Sparse Array befüllt
		id_converter meshID{};
		meshID.mask.index = free_id;
		meshID.mask.generation = mask.generation;

		return meshID.ID;
	}


	// Needs to be O(1)
	void RemoveMesh(MeshID id)
	{
		// TODO: add your implementation here.
		// thoroughly comment *what* you do, and *why* you do it (in german or english).

		// Struct generieren um auf index und generation zugreifen zu können
		id_converter meshID{id};

		// checken ob generation gleich
		id_struct entry = m_sparse_array[meshID.mask.index];

		if (entry.generation == meshID.mask.generation)
		{
			// mesh mit letzten im dense swappen um leere Einträge zu vermeiden
			std::swap(m_meshes[entry.index], m_meshes[m_meshCount-1]);
			// in sparse für geswappten mesh index aktualisieren um Zugriff für geswapptes Element wieder zu ermöglichen
			m_sparse_array[m_denseToSparse[m_meshCount-1]].index = entry.index;
			// in sparse generation erhöhen um die MeshID zu invalidieren
			m_sparse_array[m_denseToSparse[meshID.mask.index]].generation++;
			// sparse id freimachen damit sie wieder verwendet werden kann
			m_open_ids.emplace_front(static_cast<uint8_t>(meshID.mask.index));
			// denseToSparse aktualisieren um Mapping aktuell zu halten
			std::swap(m_denseToSparse[m_meshCount-1], m_denseToSparse[meshID.mask.index]);
			// m_meshCount reduzieren damit neue Meshes an die richtige Stelle geaddet werden
			m_meshCount--;
		}
	}


	// Needs to be O(1)
	Mesh* Lookup(MeshID id)
	{
		// TODO: add your implementation here.
		// thoroughly comment *what* you do, and *why* you do it (in german or english).
		
		// Struct generieren um auf index und generation zugreifen zu können
		id_converter meshID{id};

		id_struct entry = m_sparse_array[meshID.mask.index];

		//Mesh nur zurückliefern wenn generation noch gleich ist, andernfalls wurde das Objekt mit dieser MeshID bereits gelöscht/ersetzt
		if(entry.generation == meshID.mask.generation)
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
	


	//Container um alle notwendigen Daten zu speichern, deque um auch den sparse_array so gepackt wie möglich zu halten
	std::deque<uint8_t> m_open_ids;
	id_struct m_sparse_array[MAX_MESH_COUNT];
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
