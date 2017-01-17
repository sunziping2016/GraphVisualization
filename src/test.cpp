#include <iostream>
#include <fstream>
#include <sstream>
#include "graph.h"

using namespace std;

int main()
{
    graph_processor processor;
    //ifstream file("/home/sun/GraphVisualization/src/g.txt");
    //processor.load(file);
    while (true) {
        try {
            string temp_line, command;
            cout << "> ";
            getline(cin, temp_line);
            if (!cin)
                break;
            if (temp_line.empty())
                continue;
            istringstream line(temp_line);
            line >> command;
            if (command == "exit" || command == "e")
                break;
            else if (command == "load" || command == "l") {
                string filename;
                line >> filename;
                ifstream in(filename);
                if (!in)
                    throw runtime_error("Cannot open file.");
                processor.load(in);
            } else if (command == "save" || command == "s") {
                string filename;
                line >> filename;
                ofstream out(filename);
                if (!out)
                    throw runtime_error("Cannot create file.");
                processor.save(out);
            } else if (command == "clear" || command == "c") {
                processor.clear();
            } else if (command == "import" || command == "i") {
                string filename;
                line >> filename;
                ifstream in(filename);
                if (!in)
                    throw runtime_error("Cannot open file.");
                processor.import_edge_list(in);
            } else if (command == "vertex_num" || command == "vn") {
                cout << processor.get_vertex_num() << endl;
            } else if (command == "edge_list" || command == "el") {
                auto result = processor.get_edge_list();
                for (auto &edge: result)
                    cout << get<0>(edge) << " " << get<1>(edge) << " " << get<2>(edge) << endl;
            } else if (command == "shortest_path" || command == "sp") {
                size_t id1, id2;
                line >> id1 >> id2;
                if (!line)
                    throw runtime_error("Format error: shortest_path|sp <id1> <id2>");
                auto result = processor.query_shortest_path(id1, id2);
                for (auto &path: result) {
                    if (!path.empty())
                        cout << path[0];
                    for (size_t i = 1; i < path.size(); ++i)
                        cout << " -> " << path[i];
                    cout << endl;
                }
            } else if (command == "spanning_tree" || command == "st") {
                auto result = processor.query_minimum_spanning_tree();
                for (size_t i = 0; i < result.size(); ++i) {
                    if (i == result[i])
                        cout << i << endl;
                    else
                        cout << result[i] << " -> " << i << endl;
                }
            } else if (command == "betweeness_centrality" || command == "bc") {
                auto result = processor.query_betweeness_centrality();
                for (size_t i = 0; i < result.size(); ++i)
                    cout << i << ": " << result[i] << endl;
            } else if (command == "closeness_centrality" || command == "cc") {
                auto result = processor.query_closeness_centrality();
                for (size_t i = 0; i < result.size(); ++i)
                    cout << i << ": " << result[i] << endl;
            } else if (command == "components" || command == "cp") {
                auto result = processor.query_connected_components();
                for (size_t i = 0; i < result.size(); ++i) {
                    cout << i << ":";
                    for (size_t j = 0; j < result[i].size(); ++j)
                        cout << " " << result[i][j];
                    cout << endl;
                }
            } else if (command == "kk3") {
                double tolerance;
                line >> tolerance;
                if (!line)
                    throw runtime_error("Format error: kk3 <tolerance>");
                auto result = processor.layout_3dkk(true, true, tolerance);
                for (size_t i = 0; i < result.size(); ++i)
                    cout << i << ": " << result[i][0] << " " << result[i][1] << " " << result[i][2] << endl;
            } else if (command == "kk2") {
                double tolerance;
                line >> tolerance;
                if (!line)
                    throw runtime_error("Format error: kk2 <tolerance>");
                auto result = processor.layout_2dkk(true, true, tolerance);
                for (size_t i = 0; i < result.size(); ++i)
                    cout << i << ": " << result[i][0] << " " << result[i][1] << endl;
            } else if (command == "fr2" || command == "fr") {
                size_t iterations;
                line >> iterations;
                if (!line)
                    throw runtime_error("Format error: fr2|fr <iterations>");
                auto result = processor.layout_2dfr(true, true, iterations);
                for (size_t i = 0; i < result.size(); ++i)
                    cout << i << ": " << result[i][0] << " " << result[i][1] << endl;
            } else if (command == "help" || command == "h") {
                cout << "Command:\n"
                        "  exit|e                         exit the program\n"
                        "  load|l <filename>              load file\n"
                        "  save|s <filename>              save file\n"
                        "  clear|c                        clear data loaded\n"
                        "  import|i <filename>            import file as an edge list\n"
                        "  vertex_num|vn                  print number of vertice\n"
                        "  edge_list|el                   print a list of edges\n"
                        "  shortest_path|sp <id1> <id2>   print shortest paths between two points\n"
                        "  spanning_tree|st               print minimal spanning tree\n"
                        "  betweeness_centrality|bc       print betweeness centrality\n"
                        "  closeness_centrality|cc        print closeness centrality\n"
                        "  components|cp                  print connected components\n"
                        "  kk3 <tolerance>                print coordinates of 3D kamada kawai\n"
                        "  kk2 <tolerance>                print coordinates of 2D kamada kawai\n"
                        "  fr2|fr <iterations>            print coordinates of 2D Fruchterman Reingold\n"
                        "  help|h                         print this help message" << endl;
            } else
                throw runtime_error("Unknown command.");
        } catch (const exception &error) {
            cout << error.what() << endl;
        }
    }
    return 0;
}