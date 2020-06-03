#include <cstdint>
#include <cstdio>
#include <deque>

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

namespace mask_management
{
	//Benutzte struct und union um sie unabh�ngig von der Klasse zu speichern
	struct id_struct
	{
		uint32_t generation : 24,
				      index : 8;
	};

	//f�r einfachen Zugriff auf entweder die 2 einzelnen Werte oder die generelle MeshID
	union id_converter
	{
		MeshID ID;
		id_struct mask;
	};
}

// in our exercise, the RenderWorld only holds meshes, nothing more.
// the RenderWorld implementation makes sure that after *each* operation its internal data structure has no holes,
// and contains Mesh instances which are stored contiguously in memory.
class RenderWorld
{
public:
	RenderWorld(void)
		: m_meshCount(0)
	{
		//Hier wird ein uint16_t benutzt um die for Bedingung auch erf�llen zu k�nnen, mit einem uint8_t w�ren wir hier in einem unendlichen loop
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

		//n�chste freie ID im m_sparse_array wird geholt
		uint8_t free_id = m_open_ids.front();
		m_open_ids.pop_front();

		//Daten f�r sparse_array werden geschrieben: position im dense array und generation
		mask_management::id_struct mask{};
		mask.index = m_meshCount;
		mask.generation = m_sparse_array[free_id].generation;

		m_sparse_array[free_id] = mask;

		//dense to sparse array wird am index des meshes, welches das mesh auch im dense array hat,
		//mit der id f�r den Eintrag im sparse array bef�llt um Zugriff in beide Richtungen zu erm�glichen
		m_dense_to_sparse[m_meshCount] = free_id;

		// m_meshCount erh�hen damit neue Meshes an die richtige Stelle geaddet werden
		m_meshCount++;

		//MeshID wird mit der Generation und dem Index im Sparse Array bef�llt
		mask_management::id_converter meshID{};
		meshID.mask.index = free_id;
		meshID.mask.generation = mask.generation;

		return meshID.ID;
	}


	// Needs to be O(1)
	void RemoveMesh(MeshID id)
	{
		// TODO: add your implementation here.
		// thoroughly comment *what* you do, and *why* you do it (in german or english).

		// Struct generieren um auf index und generation zugreifen zu k�nnen
		mask_management::id_converter meshID{ id };

		// checken ob generation gleich
		mask_management::id_struct entry = m_sparse_array[meshID.mask.index];

		if (entry.generation == meshID.mask.generation)
		{
			// mesh mit letzten im dense swappen um leere Eintr�ge zu vermeiden
			std::swap(m_meshes[entry.index], m_meshes[m_meshCount - 1]);
			// in sparse f�r geswappten mesh index aktualisieren um Zugriff f�r geswapptes Element wieder zu erm�glichen
			m_sparse_array[m_dense_to_sparse[m_meshCount - 1]].index = entry.index;
			// in sparse generation erh�hen um die MeshID zu invalidieren
			m_sparse_array[m_dense_to_sparse[meshID.mask.index]].generation++;
			// sparse id freimachen damit sie wieder verwendet werden kann
			m_open_ids.emplace_front(static_cast<uint8_t>(meshID.mask.index));
			// denseToSparse aktualisieren um Mapping aktuell zu halten
			std::swap(m_dense_to_sparse[m_meshCount - 1], m_dense_to_sparse[meshID.mask.index]);
			// m_meshCount reduzieren damit neue Meshes an die richtige Stelle geaddet werden
			m_meshCount--;
		}
	}


	// Needs to be O(1)
	Mesh* Lookup(MeshID id)
	{
		// TODO: add your implementation here.
		// thoroughly comment *what* you do, and *why* you do it (in german or english).

		// Struct generieren um auf index und generation zugreifen zu k�nnen
		mask_management::id_converter meshID{ id };

		mask_management::id_struct entry = m_sparse_array[meshID.mask.index];

		//Mesh nur zur�ckliefern wenn generation noch gleich ist, andernfalls wurde das Objekt mit dieser MeshID bereits gel�scht/ersetzt
		if (entry.generation == meshID.mask.generation)
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

	//Container um alle notwendigen Daten zu speichern
	mask_management::id_struct m_sparse_array[MAX_MESH_COUNT];
	uint32_t m_dense_to_sparse[MAX_MESH_COUNT];

	// DO NOT CHANGE!
	// these two members are here to stay. see comments regarding Iterate().
	Mesh m_meshes[MAX_MESH_COUNT];
	unsigned int m_meshCount;

	// deque um auch den sparse_array so gepackt wie m�glich zu halten
	std::deque<uint8_t> m_open_ids;
};
