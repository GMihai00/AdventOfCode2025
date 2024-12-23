#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <set>
#include <optional>

#include "./models/Map.hpp"

std::vector<std::string> read_data(std::ifstream& in) {
    
    std::vector<std::string> rez;
    
    std::string line;
    
    while (std::getline(in , line)) {
    
        rez.push_back(line);
    }
    
    return rez;
}

long long solve_brute_force_first(const std::shared_ptr<Map> map) {

    std::vector<std::pair<int,int>> directions = {
        {-1, 0},
        {0, 1},
        {1, 0},
        {0, -1}
    };
    
    int current_direction = 0;
    
    auto current_position = map->m_guard_initial_position;
    
    int n = map->m_horizontal_view.size();
    int m = map->m_horizontal_view[0].size();  
    
    std::vector<std::vector<bool>> visited(n, std::vector<bool>(m, false));
    
    while (current_position.first < n && 
        current_position.second < m && current_position.first >= 0 && current_position.second >= 0) {
    
        visited[current_position.first][current_position.second] = true;

        int i = current_position.first + directions[current_direction].first;
        int j = current_position.second + directions[current_direction].second;
        
        // std::cout << i << " " << j << std::endl;
        
        std::pair<int, int> next = {i , j};

        if (!(next.first < n && next.second < m && i >= 0 && j >= 0)) {
            break;
        }
        
        if (map->m_horizontal_view[i][j] != '#') {
            current_position = next;
        } 
        else {
            current_direction++;
            if (current_direction == directions.size())
                current_direction = 0;
        }
        
        
    }
    
    long long rez = 0;
    
    for (int i = 0; i < visited.size(); i++)
    {
        for (int j = 0; j < visited[i].size(); j++)
        {
            if (visited[i][j] == true) 
            {   
                rez++;
                if (map->m_horizontal_view[i][j] != '^')
                    map->m_horizontal_view[i][j] = 'X';
            }
        }
    }
    
    return rez;
}


std::optional<std::pair<int, int>> get_next_blocking_object(const std::shared_ptr<Map> map, std::pair<int, int> current_poz, int current_direction, const std::vector<std::pair<int,int>>& directions)
{
    
    int line = (current_direction % 2 == 0) ? current_poz.second : current_poz.first;
    int value = (current_direction % 2 == 0) ? current_poz.first : current_poz.second;
    
    int asc;
    
    if (directions[current_direction].first != 0)
        asc = directions[current_direction].first;
    else 
        asc =  directions[current_direction].second;

    auto it = map->m_objects.at(current_direction)[line].upper_bound(value * asc);
    
    if (it != map->m_objects.at(current_direction)[line].end() && *it != value * asc) {

        std::pair<int, int> rez;
        
        if (current_direction % 2 == 0)
            rez = std::make_pair(*it / asc, line);
        else 
            rez = std::make_pair(line, *it / asc);
        
        if (map->m_horizontal_view[rez.first][rez.second] != '#')
            throw std::exception();
        
        return rez;
    }
    
    return std::nullopt;
}

bool can_form_cycle(const std::shared_ptr<Map> map, std::pair<int, int> current_poz, std::optional<std::pair<int, int>> next, int current_direction, const std::vector<std::pair<int,int>>& directions, std::map<int, std::vector<std::set<int>>> collision_obj) {
    
    // set next to be # first hand
    for (int it = 0; it < 4; it++) {
        int line = (it % 2 == 0) ? next->second : next->first;
        int value = (it % 2 == 0) ? next->first : next->second;
        
        int asc;
        
        if (directions[it].first != 0)
            asc = directions[it].first;
        else 
            asc =  directions[it].second;
    
        map->m_objects[it][line].insert(value * asc);
    }
                
    map->m_horizontal_view[next->first][next->second] = '#';
    
    auto finally = std::shared_ptr<void>(nullptr, [&map, next, directions](void*) {
        for (int it = 0; it < 4; it++) {
            int line = (it % 2 == 0) ? next->second : next->first;
            int value = (it % 2 == 0) ? next->first : next->second;
            
            int asc;
            
            if (directions[it].first != 0)
                asc = directions[it].first;
            else 
                asc =  directions[it].second;
    
            map->m_objects[it][line].erase(value * asc);
        }
        map->m_horizontal_view[next->first][next->second] = '.';
    });
    
    
    while(next != std::nullopt)
    {
        int line = (current_direction % 2 == 0) ? next->second : next->first;
        int value = (current_direction % 2 == 0) ? next->first : next->second;
        
        int asc;
        
        if (directions[current_direction].first != 0)
            asc = directions[current_direction].first;
        else 
            asc =  directions[current_direction].second;
        
        // in case we already hit the object with the current direction config we know we are in a loop
        if (collision_obj[current_direction][line].find(value * asc) != collision_obj[current_direction][line].end())
            return true;
        
        collision_obj[current_direction][line].insert(value * asc);
        
        current_poz.first = next->first - directions[current_direction].first;
        current_poz.second = next->second - directions[current_direction].second;
        
        current_direction = (current_direction + 1) % directions.size();
            
        next  = get_next_blocking_object(map, current_poz, current_direction, directions);
    }
    
    return false;
}


bool is_loop_position(const std::shared_ptr<Map> map, std::pair<int, int> current, std::pair<int, int> next, int current_direction, const std::vector<std::pair<int,int>>& directions, const std::map<int, std::vector<std::set<int>>>& collision_obj) {

    if (map->m_horizontal_view[next.first][next.second] == '^' ||  map->m_horizontal_view[next.first][next.second] == '#')
        return false;
        
    int next_direction = (current_direction + 1) % directions.size();
    
    int line = (next_direction % 2 == 0) ? current.second : current.first;
    int value = (next_direction % 2 == 0) ? current.first : current.second;
    
    int asc;
    
    if (directions[next_direction].first != 0)
        asc = directions[next_direction].first;
    else 
        asc =  directions[next_direction].second;

    auto it = collision_obj.at(next_direction)[line].upper_bound(value * asc);
    
    if (it != collision_obj.at(next_direction)[line].end() && *it != value * asc) {
        return true;
    }
    
    return can_form_cycle(map, current, next, current_direction, directions, collision_obj);
}


std::map<int, std::vector<std::set<int>>> create_collision_map(int n, int m) {
    std::vector<std::set<int>> collided_up(m, std::set<int>());
    std::vector<std::set<int>> collided_down(m, std::set<int>());
    std::vector<std::set<int>> collided_right(n, std::set<int>());
    std::vector<std::set<int>> collided_left(n, std::set<int>());
    
    std::map<int, std::vector<std::set<int>>> collided_obj;
    
    collided_obj[0] = collided_up;
    collided_obj[1] = collided_right;
    collided_obj[2] = collided_down;
    collided_obj[3] = collided_left;
    
    return collided_obj;
}


// either overcounting, or AOC answer is wrong

long long solve_second(const std::shared_ptr<Map> map) {
    
    std::vector<std::pair<int,int>> directions = {
        {-1, 0},
        {0, 1},
        {1, 0},
        {0, -1}
    };
    
    int current_direction = 0;
    
    auto current_position = map->m_guard_initial_position;
    
    int n = map->m_horizontal_view.size();
    int m = map->m_horizontal_view[0].size();  
    
    std::vector<std::vector<bool>> visited(n, std::vector<bool>(m, false));
    std::vector<std::vector<bool>> loop_generator_poz(n, std::vector<bool>(m, false));

    auto collided_obj = create_collision_map(n, m);
    
    while (current_position.first < n && 
        current_position.second < m && current_position.first >= 0 && current_position.second >= 0) {
    
        visited[current_position.first][current_position.second] = true;

        int i = current_position.first + directions[current_direction].first;
        int j = current_position.second + directions[current_direction].second;
        
        // std::cout << i << " " << j << std::endl;
        
        std::pair<int, int> next = {i , j};

        if (!(next.first < n && next.second < m && i >= 0 && j >= 0)) {
            break;
        }
        
        if (loop_generator_poz[next.first][next.second] == false && is_loop_position(map, current_position, next, current_direction, directions, collided_obj))
            loop_generator_poz[next.first][next.second] = true;
                
        if (map->m_horizontal_view[i][j] != '#') {
            current_position = next;
        } 
        else {
            
            // save object I'm colliding to
            int line = (current_direction % 2 == 0) ? j : i;
            int value = (current_direction % 2 == 0) ? i : j;
            
            int asc;
            
            if (directions[current_direction].first != 0)
                asc = directions[current_direction].first;
            else 
                asc =  directions[current_direction].second;
        
            collided_obj[current_direction][line].insert(value * asc);
            
            current_direction++;
            if (current_direction == directions.size())
                current_direction = 0;
                
        }
        
        
    }
    
    long long rez = 0;
    
    for (int i = 0; i < loop_generator_poz.size(); i++)
    {
        for (int j = 0; j < loop_generator_poz[i].size(); j++)
        {
            if (loop_generator_poz[i][j] == true) 
            {   
                rez++;
                map->m_horizontal_view[i][j] = 'O';
            }
        }
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
    
    auto data = read_data(in);
    
    auto map = std::make_shared<Map>(data);
    
    auto rez =  solve_brute_force_first(map);
    
    out << "First: " << rez << std::endl;
    
    // get list of objects hit
    
    // if I have to my right an already visited blocking object adding a circle 
    // there would mean I end up in a loop
    // it's enough to have a lowerbound / upperbound
    // only issue is that the visited direction matter
    // so 4 list visited down, up, right, left
    
    // if the visited direction is the same then I add it to the list
    
    auto rez2 = solve_second(map);
    
    out << "Second: " << rez2 << std::endl;
    
    out << *map;
}