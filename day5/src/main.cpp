#include <iostream>
#include <fstream>
#include <vector>
#include <exception>
#include <map>
#include <queue>
#include <set>

std::vector<std::pair<int, int>> get_page_ordering_rules(std::ifstream& in) {
    std::vector<std::pair<int,int>> rez;
    
    std::string line;
    
    while(std::getline(in, line)) {
        
        if (line.size() == 0){
            return rez;
        }
        
        int nr1 = 0;
        int nr2 = 0;
        bool split_found = false;
        
        for (const auto& chr : line) {
            
            if (chr == '|') {
                split_found = true;
            }
            else if (split_found){
                nr2 = nr2 * 10 + chr - '0';
            } else {
                nr1 = nr1 * 10 + chr - '0';
            }
        }
        
        rez.push_back({nr1, nr2});
    }
    
    throw std::runtime_error("No end of rule set found");
}

std::vector<std::vector<int>> build_new_graph(const std::vector<std::vector<int>>& initial_graph, const std::vector<int>& update) {

    auto clone_graph(initial_graph);
    
    // in case update contains edges not present in inital rules
    int max_value = 0;
    for (const auto& val : update) {
        max_value = std::max(val, max_value);
    }
    
    for (int i = clone_graph.size(); i <= max_value; i++)
        clone_graph.push_back({});
    
    for (int it = 0; it < update.size() - 1; it++) {
        clone_graph[update[it]].push_back(update[it + 1]);
    }
    
    // interested only in the graph containing the values of udpate
    std::set<int> update_values(update.begin(), update.end());
    
    for (int i = 0; i < clone_graph.size(); i++) {
        if (update_values.find(i) == update_values.end())
            clone_graph[i].clear();
    }
    
    return clone_graph;
}

bool visit_and_check_for_cycles(const std::vector<std::vector<int>>& graph, std::vector<bool>& visited, std::vector<bool>& rec_stack, int poz) {

    if (!visited[poz]) {
    
        visited[poz] = true;
        rec_stack[poz] = true;
        
        for (int i = 0; i < graph[poz].size(); i++)
        {
            if (!visited[graph[poz][i]] && visit_and_check_for_cycles(graph, visited, rec_stack, graph[poz][i]))
            {
                // std::cout << graph[poz][i] << std::endl;
                return true;
            }
            else if (rec_stack[graph[poz][i]])
            {
                // std::cout << graph[poz][i] << std::endl;
                return true;
            }
        }
        
    }
    
    rec_stack[poz] = false;
    return false;   
}

bool graph_contains_cycle(const std::vector<std::vector<int>>& initial_graph, const std::vector<int>& update) {

    auto clone_graph = build_new_graph(initial_graph, update);
    
    std::vector<bool> visited(clone_graph.size(), false);
    
    // parse each connected component
    for (const auto& val : update) {
        if (visited[val] == false)
        {   
            std::vector<bool> rec_stack(clone_graph.size(), false);
            if (visit_and_check_for_cycles(clone_graph, visited, rec_stack, val))
                return true;
        }
    }
    
    return false;
}


std::vector<int> get_top_sort(const std::vector<std::vector<int>>& initial_graph, const std::vector<int>& update) {

    // graph doesn't contain cycle
    
    auto clone_graph(initial_graph);
    
    std::set<int> update_values(update.begin(), update.end());
    
    for (int i = 0; i < clone_graph.size(); i++) {
        if (update_values.find(i) == update_values.end())
            clone_graph[i].clear();
    }
    
    // first top-sort the graph
    
    std::vector<bool> visited(clone_graph.size(), false);
    std::vector<int> incoming_edge_degree(clone_graph.size(), 0);
    std::vector<int> visited_order;
    
    for (int  i = 0; i < clone_graph.size(); i++) {
        for (int j = 0; j < clone_graph[i].size(); j++)
            incoming_edge_degree[clone_graph[i][j]]++;
    }
    
    std::queue<int> to_visit;
    
    for (int i = 0; i < incoming_edge_degree.size(); i++)
        if (incoming_edge_degree[i] == 0 && update_values.find(i) != update_values.end())
            to_visit.push(i);
            
    while(!to_visit.empty()){
    
        int poz = to_visit.front();
        to_visit.pop();
        
        if (update_values.find(poz) != update_values.end())
            visited_order.push_back(poz);
        
        for (int i = 0; i < clone_graph[poz].size(); i++) {
        
            int neighbour = clone_graph[poz][i];
            
            incoming_edge_degree[neighbour]--;
            
            if (incoming_edge_degree[neighbour] == 0) {
                
                to_visit.push(neighbour);
            }
        }
        
    }
    
    return visited_order;
}

std::vector<std::vector<int>> graph_mapping(const std::vector<std::pair<int,int>>& rules) {

    int max_value = 0;
    for (const auto& rule : rules) {
        max_value = std::max(max_value, rule.first);
        max_value = std::max(max_value, rule.second);
    }
    
    std::vector<std::vector<int>> rez;
    
    for (int i = 0; i <= max_value; i++)
        rez.push_back({});
    
    for (const auto& rule : rules) {
        rez[rule.first].push_back(rule.second);
    }    
    
    return rez;
}

int main() {

    std::ifstream in("../data.in");
    
    if (!in.is_open()) {
        std::cerr << "Error: Could not open the file." << std::endl;
        return 1;
    }
    
    std::ofstream out("../data.out");
    
    auto page_ordering_rules = get_page_ordering_rules(in);
    
    auto rules_graph = graph_mapping(page_ordering_rules);
    
    std::string line;
    
    long long rez = 0;
    long long rez2 = 0;
    int cnt = 0;
    
    while(std::getline(in, line)) {
        
        std::vector<int> update;
        update.clear();
        
        int nr = -1;
        
        for (const auto& chr : line) {
        
            if (nr == -1) {
                nr = 0;
            }
            
            if (chr != ',') {
                nr = nr * 10 + chr - '0';
            }
            else {
                update.push_back(nr);
                nr = -1;
            }   
        }
        
        if (nr != -1)
            update.push_back(nr);     
        
        // cycle detection of the graph should be the solution
        // if cycle exists then there is something wrong
        
        if (!graph_contains_cycle(rules_graph, update)) {
            rez += update[update.size() / 2];
            std::cout << "Line: " << cnt << " ok\n";
        }
        else {
            // good ordering is just removing your edges and parsing the rules graph as connected components
            // the problem is ambigous as you have multiple ways to solve the solution described above
            // for solving I'll just take the top-sort order
            
            auto top_sort_update = get_top_sort(rules_graph, update);
            
            rez2+= top_sort_update[update.size() / 2];
            
            std::cout << "Line: " << cnt << " not ok\n";
            
            for (int i = 0; i <  top_sort_update.size(); i++)
                std::cout << top_sort_update[i] << " ";
                
            std::cout <<"\n\n";
        }
        
        cnt ++;
    }
    
    out << "First: " << rez << std::endl;
    
    out << "Second: " << rez2 << std::endl;
    return 0;
}