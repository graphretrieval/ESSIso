/*
Copyright 2007-2012 Janez Konc

If you use this program, please cite:
Janez Konc and Dusanka Janezic. An improved branch and bound algorithm for the
maximum clique problem. MATCH Commun. Math. Comput. Chem., 2007, 58, 569-590.

More information at: http://www.sicmm.org/~konc

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/**
 * The clique computed by using this implementation maybe a disconnect forests
 * Two connected maximal clique are going to grouped into a larger clique. 
 * Thus, we use this algorithm as a base, extended it into another version:
 * 1. FindMaximumConnectedClique,  we find the maximum connected cliques by detecting the disconnected forests
 * 2. FindMaximalDisjointConnClique, based on the clique forest, we format it into a set of disjointed graphs. 
 */
#pragma once
#ifndef FIND_MAXIMUM_FOREST_CLIQUE_H
#define FIND_MAXIMUM_FOREST_CLIQUE_H

#include <iostream>
#include <algorithm>
#include <assert.h>
#include <chrono>

class FindMaximumForestClique
{
	const bool* const* e;
	int pk, level;
	std::chrono::steady_clock::time_point startExpandTime;
	const float Tlimit;
	class Vertices {
		class Vertex {
			int i, d;
		public:
			void set_i(const int ii) { i = ii; }
			int get_i() const { return i; }
			void set_degree(int dd) { d = dd; }
			int get_degree() const { return d; }
		};
		Vertex *v;
		int sz;
		static bool desc_degree(const Vertex vi, const Vertex vj) { return (vi.get_degree() > vj.get_degree()); }
	public:
#ifdef DEBUG
		void dbg_v(const string msg = "") const {
			std::cout << msg << " Vertices: [";
			for (int i = 0; i < sz; i++)
				std::cout << "(" << v[i].get_i() << "," << v[i].get_degree() << ") ";
			std::cout << "]" << std::endl;
		}
#endif
		Vertices(int size) : sz(0) { v = new Vertex[size]; }
		~Vertices() {}
		void dispose() { if (v) delete[] v; }
		void sort() { std::sort(v, v + sz, desc_degree); }
		void init_colors();
		void set_degrees(FindMaximumForestClique&);
		int size() const { return sz; }
		void push(const int ii) { v[sz++].set_i(ii); };
		void pop() { sz--; };
		Vertex& at(const int ii) const { return v[ii]; };
		Vertex& end() const { return v[sz - 1]; };
	};
	class ColorClass {
		int *i;
		int sz;
	public:
#ifdef DEBUG
		void dbg_i(const string msg = "") const {
			std::cout << msg << " Class: [";
			for (int ii = 0; ii < sz; ii++)
				std::cout << i[ii] << " ";
			std::cout << "]" << std::endl;
		}
#endif
		ColorClass() : sz(0), i(0) {}
		ColorClass(const int sz) : sz(sz), i(0) { init(sz); }
		~ColorClass() {
			if (i) delete[] i;
		}
		void init(const int sz) { i = new int[sz]; rewind(); }
		void push(const int ii) { i[sz++] = ii; };
		void pop() { sz--; };
		void rewind() { sz = 0; };
		int size() const { return sz; }
		int& at(const int ii) const { return i[ii]; }
		ColorClass& operator=(const ColorClass& dh) {
			for (int j = 0; j < dh.sz; j++) i[j] = dh.i[j];
			sz = dh.sz;
			return *this;
		}
	};
	Vertices V;
	ColorClass *C, QMAX, Q;
	class StepCount {
		int i1, i2;
	public:
		StepCount() : i1(0), i2(0) {}
		void set_i1(const int ii) { i1 = ii; }
		int get_i1() const { return i1; }
		void set_i2(const int ii) { i2 = ii; }
		int get_i2() const { return i2; }
		void inc_i1() { i1++; }
	};
	StepCount *S;
	bool connection(const int i, const int j) const { return e[i][j]; }
	bool cut1(const int, const ColorClass&);
	void cut2(const Vertices&, Vertices&);
	void color_sort(Vertices&);
	void expand(Vertices);
	void expand_dyn(Vertices);
	void _mcq(int*&, int&, bool);
	void degree_sort(Vertices &R) { R.set_degrees(*this); R.sort(); }
public:
#ifdef DEBUG
	void dbg_C() const {
		for (int i = 0; i < V.size(); i++) {
			std::cout << "C[" << i << "] : ";
			C[i].dbg_i();
		}
	}
	void dbg_conn() const {
		for (int i = 0; i < V.size(); i++) {
			for (int j = 0; j < V.size(); j++) {
				std::cout << e[i][j];
			}
			std::cout << std::endl;
		}
	}
#endif
	FindMaximumForestClique(const bool* const*, const int, const float = 0.025);
	int steps() const { return pk; }
	void mcq(int* &maxclique, int &sz) { _mcq(maxclique, sz, false); }
	void mcqdyn(int* &maxclique, int &sz) { _mcq(maxclique, sz, true); }
	~FindMaximumForestClique() {
		if (C) delete[] C;
		if (S) delete[] S;
		V.dispose();
	};
};





#endif