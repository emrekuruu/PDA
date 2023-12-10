#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<tuple>
#include<fstream>
#include <unordered_set>
#include <stack>

using namespace std;

void print(const auto& container){
    for(const auto& item: container)
        cout << item << " ";
    cout<<endl;
}


void print(const std::map<tuple<string,char,char,char>, vector<string>>& myMap) {

    for (const auto& pair : myMap) {
        for (const auto& next :  pair.second){
            std::cout << "From:" << std::get<0>(pair.first) << ", With:" << std::get<1>(pair.first)
                      << ", Pop:" << std::get<2>(pair.first) << ", Push:" << std::get<3>(pair.first) << " Go to: "
                      << next << std::endl;
        }
    }
}

struct PDA{

    stack<char> pda_stack;

    int number_of_states;

    int number_of_input_variables;

    int number_of_stack_variables;

    int number_of_finals;

    vector<string> states;

    unordered_set<string> final_states;

    vector<char> input_variables;

    vector<char> stack_variables;

    // (current_state , action , pop , push) -> next_state
    map<tuple<string,char, char, char>,vector<string>> transitions;

    string current;

    string start;

    char initial_stack_symbol;

    vector<string> actions;

    PDA(const string& filename){

        ifstream file(filename);

        if (!file.is_open()) {
            cerr << "Failed to open the file: " << filename << endl;
        }

        else {
            file >> this->number_of_input_variables;
            file >> this->number_of_stack_variables;
            file >> this->number_of_finals;
            file >> this->number_of_states;

            for(int i = 0; i < this->number_of_states; i++){
                string state;
                file >> state;
                this->states.push_back(state);
            }

            file >> current;

            this->start = current;

            for(int i = 0; i < this->number_of_finals; i++){
                string final;
                file >> final;
                this->final_states.insert(final);
            }

            for(int i = 0; i < this->number_of_stack_variables; i++){
                char variable;
                file >> variable;
                this->stack_variables.push_back(variable);
            }

            file >> this->initial_stack_symbol;

            for(int i = 0; i < this->number_of_input_variables; i++){
                char variable;
                file >> variable;
                this->input_variables.push_back(variable);
            }

            while(true){
                string current;
                char action;
                char pop;
                char push;
                string next;

                file >> current;

                if(current == "END"s){
                    break;
                }

                file >> action;
                file >> pop;
                file >> push;
                file >> next;

                this->transitions[std::make_tuple(current, action,pop,push)].push_back(next);
            }
            string action;
            while(!file.eof()){
                file >> action;
                this->actions.push_back(action);
            }
            this->actions.pop_back();

            file.close();
        }
        cout <<"PDA created"<<endl;
        this->printPDA();
    }

    void printPDA() const {

        cout << this->number_of_input_variables << endl;
        cout << this->number_of_stack_variables << endl;
        cout << this->number_of_finals << endl;
        cout << this->number_of_states << endl;
        print(this->states);
        cout << this->current << endl;
        print(this->final_states);
        print(this->stack_variables);
        cout << this->initial_stack_symbol << endl;
        print(this->input_variables);
        print(this->transitions);
        print(this->actions);
        cout<<endl;
    }

    tuple<bool, bool, vector<string>> runHelper(const string& input, int position, vector<string>& path) {
        // Debugging: Print current path
        cout << "Current Path: ";
        for (const auto& state : path) {
            cout << state << " ";
        }
        cout << endl;

        // Base case: Check if the input is fully processed, the stack is empty, and the current state is final
        if (input.size() == position && this->pda_stack.empty() && this->final_states.count(this->current) > 0) {
            cout << "Accepted: Reached final state with empty stack and full input processed." << endl;
            return {true, false, path};
        }

        char nextInput = position < input.size() ? input[position] : 'E'; // 'E' for empty input
        bool anyTransitionPossible = false;

        // Explore all possible transitions from the current state
        for (const auto& trans : this->transitions) {
            auto &[state, inputChar, pop, push] = trans.first;

            // Check if the transition is applicable
            if (state == this->current && (inputChar == nextInput || inputChar == 'E') && ( pop == this->pda_stack.top() || pop == 'E')) {
                anyTransitionPossible = true;

                for (const auto &next: trans.second) {

                    string oldState = this->current;
                    char oldPop = pop;
                    char oldPush = push;

                    // Apply transition
                    this->current = next;
                    cout << "Transition: " << state << " -> " << next << " with input '" << inputChar << "'" << endl;

                    if (pop != 'E' && !this->pda_stack.empty()) {
                        this->pda_stack.pop();
                        cout << "Popped: " << pop << endl;
                    }
                    else { cout << "Popped Nothing"<<endl; }

                    if (push != 'E') {
                        this->pda_stack.push(push);
                        cout << "Pushed: " << push << endl;
                    }
                    else { cout << "Pushed Nothing"<<endl; }

                    // Update path
                    path.push_back(this->current);

                    // Decide whether to consume an input character
                    int nextPosition = position + (inputChar != 'E' ? 1 : 0);

                    // Recurse
                    auto [accepted, stuck, finalPath] = runHelper(input, nextPosition, path);

                    if (accepted) {
                        return {true, false, finalPath};
                    }

                    // Undo the transition if not accepted
                    path.pop_back();

                    this->current = oldState;

                    cout << "Backtracking: Reverting to state " << oldState << endl;

                    if (oldPush != 'E') {
                        if (!this->pda_stack.empty()) {
                            this->pda_stack.pop();
                            cout << "Backtrack Pop: " << oldPush << endl;
                        } else {
                            cout << "Warning: Attempted to backtrack pop from an empty stack" << endl;
                        }
                    }

                    if (oldPop != 'E') {
                        this->pda_stack.push(oldPop);
                        cout << "Backtrack Push: " << oldPop << endl;
                    }

                }
            }
        }

        // If no transitions are possible, the PDA is stuck
        if (!anyTransitionPossible) {
            cout << "Stuck: No possible transitions from state " << this->current << endl;
        }

        return {false, !anyTransitionPossible, path};
    }

    tuple<bool,bool, vector<string>> rejected_path(const string& input, int position, vector<string>& path) {

        if(pda_stack.empty()){
            // Check all possible transitions
            for (auto &trans : this->transitions) {
                auto &[state, inputChar, pop, push] = trans.first;
                if (state == current && inputChar == 'E' && pop =='E' && push == 'E') {
                    for (const auto &next: trans.second) {
                        // Apply transition
                        string oldState = current;
                        current = next;
                        // Update path
                        path.push_back(this->current);
                        // Recurse
                        auto [accepted, stuck, finalPath] = rejected_path(input, position, path);
                        // Undo the transition if not accepted
                        path.pop_back();
                        this->current = oldState;
                    }
                }
            }
            return {false,true,path};
        }

        char nextInput = position < input.size() ? input[position] : 'E';

        // Check all possible transitions
        for (auto &trans : this->transitions) {

            auto &[state, inputChar, pop, push] = trans.first;

            if (state == current && (inputChar == nextInput || inputChar == 'E') && ( pop == this->pda_stack.top() || pop == 'E') ) {

                for (const auto &next: trans.second) {
                    // Apply transition
                    string oldState = current;
                    current = next;

                    // Decide whether to consume an input character
                    int nextPosition = position;
                    if (inputChar != 'E') {
                        nextPosition++;
                    }

                    if (pop != 'E') {
                        this->pda_stack.pop();
                    }

                    if (push != 'E') {
                        this->pda_stack.push(push);
                    }

                    path.push_back(this->current);

                    // Recurse
                    auto [accepted, stuck, finalPath] = rejected_path(input, nextPosition, path);

                    if (stuck)
                        return {false, true, path};

                    // Undo the transition if not accepted and not stuck ( i.e the state wasn't able to consume all the inputs )
                    path.pop_back();
                    this->current = oldState;

                    if (push != 'E')
                        this->pda_stack.pop();

                    if (pop != 'E')
                        this->pda_stack.push(pop);

                }
            }
        }
        return {false,true,path};
    }

    vector<string> reset(){
        this->pda_stack = stack<char>();
        this->current = this->start;
        auto path = vector<string>{this->current};
        this->pda_stack.push(this->initial_stack_symbol);
        this->current = this->transitions[make_tuple(this->current,'E','E',this->pda_stack.top())][0];
        path.push_back(this->current);
        return path;
    }

    tuple<bool, vector<string>> operator()(const string& input) {
        cout<<endl;
        cout<<"Simulating for: "<<input<<endl;
        auto path = reset();
        auto [accepted,stuck,path_] = runHelper(input, 0, path);
        if(accepted)
            return {accepted, path_};
        else{
            path = reset();
            auto [rejected,stuck_,visited] = rejected_path(input, 0, path);
            return {rejected,visited};
        }
    }
};


struct Simulator {

    Simulator() = default;

    void operator()(PDA& pda){
        //The path to your input file
        ofstream output("output.txt"s);
        // Check if the file opened successfully
        if (output.is_open()) {

            if(pda.actions.empty()){
                auto accepted = (pda.final_states.count(pda.current) > 0);
                cout << (accepted ? "Accepted" : "Rejected")<<" ";
                output << "Empty String" << endl;
                output <<  (accepted? "Accepted" : "Rejected") << endl;
            }

            for(auto& action :  pda.actions){

                auto [accepted,path] = pda(action);

                for(const auto& state : path)
                    output << state <<" ";

                output<<endl;
                output <<  ( (accepted) ? "Accepted" : "Rejected" )  << endl;
            }
            output.close();
        }
        else {
            std::cerr << "Failed to open the file for writing." << std::endl;
        }
    }

};



int main(){
    //The path to your input file
    auto input="input.txt"s;

    auto pda = PDA(input);

    auto simulator = Simulator();

    simulator(pda);

    return 0;

}

