// Copyright (c) 2005  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source$
// $Revision$ $Date$
// $Name$
//
// Author(s)     : Abdelkrim Mebarki <Abdelkrim.Mebarki@sophia.inria.fr>

#ifndef CGAL_TRIANGULAR_FIELD_2_H_ 
#define CGAL_TRIANGULAR_FIELD_2_H_

#include <CGAL/basic.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Polygon_2_algorithms.h>

#include <float.h>
// #include <strings.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <list>
#include <queue>
#include <cmath> 
#include <string>

#include <CGAL/Triangulation_face_base_with_info_2.h>

CGAL_BEGIN_NAMESPACE

template <class StreamLinesTraits_2>
class Triangular_field_2{
public:
	typedef Triangular_field_2<StreamLinesTraits_2> Vector_field_2;
	typedef StreamLinesTraits_2 Geom_traits;
	typedef typename StreamLinesTraits_2::FT FT;
	typedef typename StreamLinesTraits_2::Point_2 Point_2;
	typedef typename StreamLinesTraits_2::Vector_2 Vector_2;
protected:
	typedef CGAL::Triangulation_vertex_base_2<StreamLinesTraits_2> Vb;
	typedef CGAL::Triangulation_face_base_with_info_2<Vector_2, StreamLinesTraits_2> Fb;
	typedef CGAL::Triangulation_data_structure_2<Vb,Fb> TDS;
	typedef CGAL::Delaunay_triangulation_2<StreamLinesTraits_2,TDS> D_Ttr;
	typedef typename D_Ttr::Vertex_handle Vertex_handle;
	typedef typename TDS::Face_handle Face_handle;
public:
	typedef typename D_Ttr::Vertex_iterator Vertex_iterator;
	D_Ttr m_D_Ttr;
protected:
	Vector_2 get_vector_field(const Point_2 & p) const;
	FT get_density_field(const Point_2 & p) const;
	void fill(std::ifstream & f);
public:
	Triangular_field_2(std::ifstream & f){
	fill(f);}
	inline typename Geom_traits::Iso_rectangle_2 iso_rectangle() const;
	std::pair<Vector_2,FT> get_field(const Point_2 & p) const;
	bool is_in_domain(const Point_2 & p) const;
	FT get_integration_step(const Point_2 &) const;
	FT get_integration_step() const;
protected:
	FT minx;
	FT miny;
	FT maxx;
	FT maxy;
protected:
	mutable std::map<Point_2, Vector_2> field_map;
	FT distance(const Point_2 & p, const Point_2 & q){
		return sqrt(((p.x() - q.x()) * (p.x() - q.x())) + ((p.y() - q.y()) * (p.y() - q.y())));};
};

template <class StreamLinesTraits_2>
void
Triangular_field_2<StreamLinesTraits_2>::fill(std::ifstream & f){
	std::cout << "reading file...\n";
	int number_of_vertices;
	f >> number_of_vertices;
	for (int i=0;i<number_of_vertices;i++){
		Point_2 p;
		Vector_2 v;
		f >> p;
		f >> v;
		if (v.x()!=0 && v.y()!=0){
		p = Point_2(p.x()*200, p.y()*200);
		Vertex_handle m_Vertex_handle = m_D_Ttr.insert(p);
		field_map[p] = v;
		if (m_D_Ttr.number_of_vertices() == 1){
			maxx = minx = p.x();
			maxy = miny = p.y();}
		if(p.x()<minx)
			minx = p.x();
		if(p.y()<miny)
			miny = p.y();
		if(p.x()>maxx)
			maxx = p.x();
		if(p.y()>maxy)
			maxy = p.y();}}
			std::cout << "number of samples " << m_D_Ttr.number_of_vertices() << "\n";}

template <class StreamLinesTraits_2> 
inline
typename Triangular_field_2<StreamLinesTraits_2>::Geom_traits::Iso_rectangle_2
Triangular_field_2<StreamLinesTraits_2>::iso_rectangle() const{
	return typename Geom_traits::Iso_rectangle_2(minx, miny, maxx, maxy);};

template <class StreamLinesTraits_2>
bool 
Triangular_field_2<StreamLinesTraits_2>::is_in_domain(const Point_2 & p) const{
	Face_handle f = m_D_Ttr.locate(p);
	return !m_D_Ttr.is_infinite(f);}

template <class StreamLinesTraits_2>
typename std::pair<typename Triangular_field_2<StreamLinesTraits_2>::Vector_2, typename Triangular_field_2<StreamLinesTraits_2>::FT>
Triangular_field_2<StreamLinesTraits_2>::get_field(const Point_2 & p) const{
	assert(is_in_domain(p));
	Vector_2 v = get_vector_field(p);
	FT density = get_density_field(p);
	std::pair<Vector_2, FT> field_value(v,density);
	return field_value;}

template <class StreamLinesTraits_2>
typename Triangular_field_2<StreamLinesTraits_2>::Vector_2 
Triangular_field_2<StreamLinesTraits_2>::get_vector_field(const Point_2 & p) const{
	Face_handle m_Face_handle = m_D_Ttr.locate(p);
	assert(is_in_domain(p));
	Point_2 p0 = m_Face_handle->vertex(0)->point();
	Point_2 p1 = m_Face_handle->vertex(1)->point();
	Point_2 p2 = m_Face_handle->vertex(2)->point();
	Vertex_handle v0 = m_Face_handle->vertex(0);
	Vertex_handle v1 = m_Face_handle->vertex(1);
	Vertex_handle v2 = m_Face_handle->vertex(2);
	FT s0,s1,s2,s;
	std::vector<Point_2> vec;
	vec.push_back(p0); vec.push_back(p1); vec.push_back(p2);
	s = polygon_area_2(vec.begin(), vec.end(), m_D_Ttr.geom_traits());
	vec.clear();
	vec.push_back(p); vec.push_back(p1); vec.push_back(p2);
	s0 = polygon_area_2(vec.begin(), vec.end(), m_D_Ttr.geom_traits());
	vec.clear();
	vec.push_back(p0); vec.push_back(p); vec.push_back(p2);
	s1 = polygon_area_2(vec.begin(), vec.end(), m_D_Ttr.geom_traits());
	vec.clear();
	vec.push_back(p0); vec.push_back(p1); vec.push_back(p);
	s2 = polygon_area_2(vec.begin(), vec.end(), m_D_Ttr.geom_traits());
	vec.clear(); 
	s0 = s0 / s; s1 = s1 / s; s2 = s2 / s;
	Vector_2 v_0 = field_map[p0];  
	Vector_2 v_1 = field_map[p1];
	Vector_2 v_2 = field_map[p2];
	FT x = ((v_0.x()*s0)+(v_1.x()*s1)+(v_2.x()*s2));
	FT y = ((v_0.y()*s0)+(v_1.y()*s1)+(v_2.y()*s2));
	FT normal = sqrt((x)*(x) + (y)*(y));
	if (normal != 0){
		x = x / normal;
		y = y / normal;}
	Vector_2  v = Vector_2(x, y);
return v;}

template <class StreamLinesTraits_2>
typename Triangular_field_2<StreamLinesTraits_2>::FT
Triangular_field_2<StreamLinesTraits_2>::get_density_field(const Point_2 & p) const{
	return p.x();}

template<class StreamLinesTraits_2>
typename Triangular_field_2<StreamLinesTraits_2>::FT
Triangular_field_2<StreamLinesTraits_2>::get_integration_step(const Point_2 &) const{
	return 1.0;}

template<class StreamLinesTraits_2>
typename Triangular_field_2<StreamLinesTraits_2>::FT
Triangular_field_2<StreamLinesTraits_2>::get_integration_step() const{
	return 1.0;}

CGAL_END_NAMESPACE

#endif
