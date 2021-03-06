#ifndef __trimesh_h__
#define __trimesh_h__

#include "trimesh_types.h" // triangle_t, edge_t
#include <vector>
#include <map>
#include <algorithm>

namespace trimesh
{

	// trimesh_t::build() needs the unordered edges of the mesh.  If you don't have them, call this first.
	void unordered_edges_from_triangles(const unsigned long num_triangles, const trimesh::triangle_t* triangles, std::vector< trimesh::edge_t >& edges_out);

	class trimesh_t
	{
	public:
		// I need positive and negative numbers so that I can use -1 for an invalid index.
		typedef long index_t;

		struct halfedge_t
		{
			// Index into the vertex array. Mostly for debug and ease of use. Can remove
			index_t from_vertex;
			// Index into the vertex array.
			index_t to_vertex;
			// Index into the face array.
			index_t face;
			// Index into the edges array.
			index_t edge;
			// Index into the halfedges array.
			index_t opposite_he;
			// Index into the halfedges array.
			index_t next_he;
			// Index into the halfedges array, instead of next_he if gap is present
			index_t ghost_he;

			halfedge_t() :
				from_vertex(-1),
				to_vertex(-1),
				face(-1),
				edge(-1),
				opposite_he(-1),
				next_he(-1),
				ghost_he(-1)
			{}

			~halfedge_t() {};
		};

		// Builds the half-edge data structures from the given triangles and edges.
		// NOTE: 'edges' can be derived from 'triangles' by calling
		//       unordered_edges_from_triangles(), above.  build() does not
		//       but could do this for callers who do not already have edges.
		// NOTE: 'triangles' and 'edges' are not needed after the call to build()
		//       completes and may be destroyed.
		void build(const unsigned long num_vertices, const unsigned long num_triangles, const trimesh::triangle_t* triangles, const unsigned long num_edges, const trimesh::edge_t* edges);

		void clear()
		{
			m_halfedges.clear();
			m_vertex_halfedges.clear();
			m_face_halfedges.clear();
			m_edge_halfedges.clear();
			m_directed_edge2he_index.clear();
		}

		const halfedge_t& halfedge(const index_t i) const { return m_halfedges.at(i); }

		std::pair< index_t, index_t > he_index2directed_edge(const index_t he_index) const
		{
			/*
			Given the index of a halfedge_t, returns the corresponding directed edge (i,j).

			untested
			*/

			const halfedge_t& he = m_halfedges[he_index];
			return std::make_pair(m_halfedges[he.opposite_he].to_vertex, he.to_vertex);
		}

		index_t directed_edge2he_index(const index_t i, const index_t j) const
		{
			/*
			Given a directed edge (i,j), returns the index of the 'halfedge_t' in
			halfedges().

			untested
			*/

			/// This isn't const, and doesn't handle the case where (i,j) isn't known:
			// return m_directed_edge2he_index[ std::make_pair( i,j ) ];

			directed_edge2index_map_t::const_iterator result = m_directed_edge2he_index.find(std::make_pair(i, j));
			if (result == m_directed_edge2he_index.end()) return -1;

			return result->second;
		}

		bool vertex_in_face(const index_t vertex_index, const index_t face) const
		{
			bool ret = false;
			if (face != -1)
			{
				index_t hei = m_face_halfedges[face];
				for (size_t i = 0; i < 3; i++)
				{
					if (m_halfedges[hei].to_vertex == vertex_index)
					{
						ret = true;
					}
					hei = m_halfedges[hei].next_he;
				}
			}
			return ret;
		}

		bool vertex_connected(const index_t vertex_one, const index_t vertex_two) const
		{
			if (vertex_one == vertex_two)
			{
				return true;
			}
			directed_edge2index_map_t::const_iterator it;
			it = m_directed_edge2he_index.find(std::make_pair(vertex_one, vertex_two));
			if (it != m_directed_edge2he_index.cend())
			{
				return true;
			}
			it = m_directed_edge2he_index.find(std::make_pair(vertex_two, vertex_one));
			if (it != m_directed_edge2he_index.cend())
			{
				return true;
			}
			return false;
		}


		void vertex_vertex_neighbors(const index_t vertex_index, std::vector< index_t >& result) const
		{
			/*
			Returns in 'result' the vertex neighbors (as indices) of the vertex 'vertex_index'.

			untested
			*/

			result.clear();

			const index_t start_hei = m_vertex_halfedges[vertex_index];
			index_t hei = start_hei;
			while (true)
			{
				bool added = false;
				const halfedge_t& he = m_halfedges[hei];
				if (vertex_connected(vertex_index, he.to_vertex))
				{
					result.push_back(he.to_vertex);
					added = true;
				}
				if (he.opposite_he == -1)
				{
					hei = he.ghost_he;
					if (vertex_connected(vertex_index, m_halfedges[hei].from_vertex) && added == false)
					{
						result.push_back(m_halfedges[hei].from_vertex);
						added = true;
					}
					hei = m_halfedges[hei].next_he;
				}
				else
				{
					hei = m_halfedges[he.opposite_he].next_he;
				}

				if (hei == start_hei) break;
			}
		}
		std::vector< index_t > vertex_vertex_neighbors(const index_t vertex_index) const
		{
			std::vector< index_t > result;
			vertex_vertex_neighbors(vertex_index, result);
			return result;
		}

		void vertices_for_face(const index_t face, std::vector< index_t>& res) const
		{
			res.clear();
			index_t hei = m_face_halfedges[face];
			for (size_t i = 0; i < 3; i++)
			{
				res.push_back(m_halfedges[hei].to_vertex);
				hei = m_halfedges[hei].next_he;
			} 
		}

		std::vector< index_t > vertices_for_face(const index_t face) const
		{
			std::vector< index_t > result;
			vertices_for_face(face, result);
			return result;
		}

		int vertex_valence(const index_t vertex_index) const
		{
			/*
			Returns the valence (number of vertex neighbors) of vertex with index 'vertex_index'.

			untested
			*/

			std::vector< index_t > neighbors;
			vertex_vertex_neighbors(vertex_index, neighbors);
			return neighbors.size();
		}

		void vertex_face_neighbors(const index_t vertex_index, std::vector< index_t >& result) const
		{
			/*
			Returns in 'result' the face neighbors (as indices) of the vertex 'vertex_index'.

			untested
			*/

			result.clear();

			const index_t start_hei = m_vertex_halfedges[vertex_index];
			index_t hei = start_hei;
			while (true)
			{
				const halfedge_t& he = m_halfedges[hei];
				if (vertex_in_face(vertex_index, he.face)) result.push_back(he.face);

				if (he.opposite_he != -1)
				{
					hei = m_halfedges[he.opposite_he].next_he;
				}
				else
				{
					hei = m_halfedges[he.ghost_he].next_he;
				}
				if (hei == start_hei) break;
			}
		}
		void vertex_face_neighbors_quad(const index_t vertex_index, std::vector< index_t >& result) const
		{
			/*Untested*/
			vertex_face_neighbors(vertex_index, result);
			auto min_face = std::min_element(result.begin(), result.end());
			auto max_face = std::max_element(result.begin(), result.end());
			if (*min_face != 0)
			{
				result.push_back(*min_face - 1);
			}
			if (*max_face != m_face_halfedges.size()-1)
			{
				result.push_back(*max_face + 1);
			}
		}
		void vertex_face_neighbors_ignore_holes(const index_t vertex_index, std::vector<index_t>& result) const
		{
			/*
			returns in 'result' the face neighbors (as indices) of the vertex 'vertex_index' ignoring holes

			untested
			*/

			result.clear();

			const index_t start_hei = m_vertex_halfedges[vertex_index];
			index_t hei = start_hei;
			while (true)
			{
				const halfedge_t& he = m_halfedges[hei];
				if (-1 != he.face) result.push_back(he.face);
				if (he.opposite_he == -1 && he.ghost_he != -1)
				{
					hei = m_halfedges[he.ghost_he].next_he;
				}
				else
				{
					hei = m_halfedges[he.opposite_he].next_he;
				}
				if (hei == start_hei) break;
			}
		}

		std::vector<index_t> vertex_face_neighbors_ignore_holes(const index_t vertex_index) const
		{
			std::vector<index_t> res;
			vertex_face_neighbors_ignore_holes(vertex_index, res);
			return res;
		}

		std::vector< index_t > vertex_face_neighbors(const index_t vertex_index) const
		{
			std::vector< index_t > result;
			vertex_face_neighbors(vertex_index, result);
			return result;
		}

		bool vertex_is_boundary(const index_t vertex_index) const
		{
			/*
			Returns whether the vertex with given index is on the boundary.

			untested
			*/

			return -1 == m_halfedges[m_vertex_halfedges[vertex_index]].face;
		}

		std::vector< index_t > boundary_vertices() const;

		std::vector< std::pair< index_t, index_t > > boundary_edges() const;

		const halfedge_t& face2he(const index_t face) const
		{
			index_t index = m_face_halfedges.at(face);
			return m_halfedges.at(index);
		}

		void split_vertex(index_t vertex_to_split, std::vector<index_t> faces_above, std::vector<index_t> faces_below);

		void common_vertices_for_faces(std::vector<index_t>& ret, index_t face1, index_t face2) const
		{
			/* Untested */
			ret.clear();
			std::vector<index_t> vert1 = vertices_for_face(face1);
			std::vector<index_t> vert2 = vertices_for_face(face2);
			for (const index_t i : vert1)
			{
				auto found = std::find(vert2.cbegin(), vert2.cend(), i);
				if (found != vert2.cend())
				{
					ret.push_back(i);
				}
			}
		}

		std::vector<index_t> common_vertices_for_faces(index_t face1, index_t face2) const
		{
			/* Untested */
			std::vector<index_t> ret;
			common_vertices_for_faces(ret, face1, face2);
			return ret;
		}

		void get_indices(std::vector<index_t>& indices);
		std::vector<unsigned short> get_indices();
	private:
		std::vector< halfedge_t > m_halfedges;
		// Offsets into the 'halfedges' sequence, one per vertex.
		std::vector< index_t > m_vertex_halfedges;
		// Offset into the 'halfedges' sequence, one per face.
		std::vector< index_t > m_face_halfedges;
		// Offset into the 'halfedges' sequence, one per edge (unordered pair of vertex indices).
		std::vector< index_t > m_edge_halfedges;
		// A map from an ordered edge (an std::pair of index_t's) to an offset into the 'halfedge' sequence.
		typedef std::map< std::pair< index_t, index_t >, index_t > directed_edge2index_map_t;
		directed_edge2index_map_t m_directed_edge2he_index;

		std::vector<index_t> he_to_unlink(std::vector<index_t> faces_above, std::vector<index_t> faces_below);

		void update_directed_edges(std::pair<index_t, index_t> old_edge, std::pair<index_t, index_t> new_edge);
	};

}

#endif /* __trimesh_h__ */
