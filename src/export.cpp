/*!
 * \file    export.cpp
 * \brief   Functions to be exported and called by the front-end.
 * \author  Sun Ziping
 * \date    2017/1/11
 */
#include <iostream>
#include <fstream>
#include <string>

#include "graph.h"
#include "export.h"

using namespace std;

DllExport const char *str_get(const string *str)
{
    return str->c_str();
}
DllExport void str_free(string *str)
{
    delete str;
}


DllExport graph_processor *graph_ctor()
{
    return new graph_processor;
}

DllExport void graph_dtor(graph_processor *ins)
{
    delete ins;
}

DllExport string *graph_load(graph_processor *ins, const char *filename)
{
    try {
        ifstream file(filename);
        if (!file)
            throw runtime_error("Cannot open file.");
        ins->load(file);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    return new string("{}");
}

DllExport string *graph_save(graph_processor *ins, const char *filename)
{
    try {
        ofstream file(filename);
        if (!file)
            throw runtime_error("Cannot create file.");
        ins->save(file);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    return new string("{}");
}

DllExport string *graph_clear(graph_processor *ins)
{
    try {
        ins->clear();
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    return new string("{}");
}


DllExport string *graph_import_edge_list(graph_processor *ins, const char *filename)
{
    try {
        ifstream file(filename);
        if (!file)
            throw runtime_error("Cannot open file.");
        ins->import_edge_list(file);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    return new string("{}");
}

DllExport string *graph_get_vertex_num(graph_processor *ins)
{
    size_t num = 0;
    try {
        num = ins->get_vertex_num();
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    return new string("{\"result\":" + to_string(num) + "}");
}

DllExport string *graph_get_edge_list(graph_processor *ins)
{
    vector<tuple<size_t, size_t, double> > edges;
    try {
        edges = ins->get_edge_list();
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    if (!edges.empty()) {
        result->append("[" + to_string(get<0>(edges.front())) + "," + to_string(get<1>(edges.front())) + "," + to_string(get<2>(edges.front())) + "]");
        for (vector<tuple<size_t, size_t, double> >::iterator iter = edges.begin(); ++iter != edges.end();)
            result->append(",[" + to_string(get<0>(*iter)) + "," + to_string(get<1>(*iter)) + "," + to_string(get<2>(*iter)) + "]");
    }
    result->append("]}");
    return result;
}

DllExport string *graph_get_vertex_property(graph_processor *ins, size_t id)
{
    graph_processor::vertex_property_type p;
    try {
        p = ins->get_vertex_property(id);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    return new string("{\"result\":{}}");
}

DllExport string *graph_get_edge_property(graph_processor *ins, size_t id1, size_t id2)
{
    graph_processor::edge_property_type p;
    try {
        p = ins->get_edge_property(id1, id2);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    return new string("{\"result\":{}}");
}

DllExport string *graph_query_shortest_path(graph_processor *ins, size_t id1, size_t id2)
{
    vector<vector<size_t> > shortest_paths;
    try {
        shortest_paths = ins->query_shortest_path(id1, id2);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    for (size_t i = 0; i < shortest_paths.size(); ++i) {
        if (i != 0)
            result->append(",");
        result->append("[");
        for (size_t j = 0; j < shortest_paths[i].size(); ++j) {
            if (j != 0)
                result->append(",");
            result->append(to_string(shortest_paths[i][j]));
        }
        result->append("]");
    }
    result->append("]}");
    return result;
}

DllExport string *graph_query_minimum_spanning_tree(graph_processor *ins) {
    vector<size_t> spanning_tree;
    try {
        spanning_tree = ins->query_minimum_spanning_tree();
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    for (size_t i = 0; i < spanning_tree.size(); ++i) {
        if (i != 0)
            result->append(",");
        result->append(to_string(spanning_tree[i]));
    }
    result->append("]}");
    return result;
}

DllExport string *graph_query_betweeness_centrality(graph_processor *ins) {
    vector<size_t> betweeness_centrality;
    try {
        betweeness_centrality = ins->query_betweeness_centrality();
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    for (size_t i = 0; i < betweeness_centrality.size(); ++i) {
        if (i != 0)
            result->append(",");
        result->append(to_string(betweeness_centrality[i]));
    }
    result->append("]}");
    return result;
}

DllExport string *graph_query_closeness_centrality(graph_processor *ins) {
    vector<double> closeness_centrality;
    try {
        closeness_centrality = ins->query_closeness_centrality();
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    for (size_t i = 0; i < closeness_centrality.size(); ++i) {
        if (i != 0)
            result->append(",");
        result->append(to_string(closeness_centrality[i]));
    }
    result->append("]}");
    return result;
}

DllExport string *graph_query_connected_components(graph_processor *ins)
{
    vector<vector<size_t> > components;
    try {
        components = ins->query_connected_components();
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    for (size_t i = 0; i < components.size(); ++i) {
        if (i != 0)
            result->append(",");
        result->append("[");
        for (size_t j = 0; j < components[i].size(); ++j) {
            if (j != 0)
                result->append(",");
            result->append(to_string(components[i][j]));
        }
        result->append("]");
    }
    result->append("]}");
    return result;
}

DllExport string *graph_layout_3dkk(graph_processor *ins, bool reinitial, bool recalculate, double tolerance)
{
    std::vector<boost::cube_topology<>::point_type> c;
    try {
        c = ins->layout_3dkk(reinitial, recalculate, tolerance);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    if (!c.empty()) {
        result->append("[" + to_string(c.front()[0]) + "," + to_string(c.front()[1]) + "," + to_string(c.front()[2]) + "]");
        for (vector<boost::cube_topology<>::point_type>::iterator iter = c.begin(); ++iter != c.end();)
            result->append(",[" + to_string((*iter)[0]) + "," + to_string((*iter)[1]) + "," + to_string((*iter)[2]) + "]");
    }
    result->append("]}");
    return result;
}

DllExport string *graph_layout_2dkk(graph_processor *ins, bool reinitial, bool recalculate, double tolerance)
{
    std::vector<boost::square_topology<>::point_type> c;
    try {
        c = ins->layout_2dkk(reinitial, recalculate, tolerance);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    if (!c.empty()) {
        result->append("[" + to_string(c.front()[0]) + "," + to_string(c.front()[1]) + ",0]");
        for (vector<boost::square_topology<>::point_type>::iterator iter = c.begin(); ++iter != c.end();)
            result->append(",[" + to_string((*iter)[0]) + "," + to_string((*iter)[1]) + ",0]");
    }
    result->append("]}");
    return result;
}

DllExport string *graph_layout_2dfr(graph_processor *ins, bool reinitial, bool recalculate, size_t iterations)
{
    std::vector<boost::square_topology<>::point_type> c;
    try {
        c = ins->layout_2dfr(reinitial, recalculate, iterations);
    } catch (const exception &error) {
        return new string("{\"error\":\"" + str_escape(error.what()) + "\"}");
    }
    string *result = new string("{\"result\":[");
    if (!c.empty()) {
        result->append("[" + to_string(c.front()[0]) + "," + to_string(c.front()[1]) + ",0]");
        for (vector<boost::square_topology<>::point_type>::iterator iter = c.begin(); ++iter != c.end();)
            result->append(",[" + to_string((*iter)[0]) + "," + to_string((*iter)[1]) + ",0]");
    }
    result->append("]}");
    return result;
}
