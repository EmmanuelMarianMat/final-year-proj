// C header files
#include <iostream>
#include <string>
#include <algorithm> 
#include <vector>
#include <unordered_map>

// Include guard
#ifndef CLASS_DEFINITIONS_H_
#define CLASS_DEFINITIONS_H_

using namespace std;

unordered_map<string,int> OpType = {
    {"OR",0},
    {"AND",1},
    {"ATTR",2},
    {"THRESHOLD",3},
    {"CONDITIONAL",4},
    {"NONE",5}
};

class BinNode{
    public:
        // string value;
        string node_type;
        string attribute;
        int index;
        bool negated;
        BinNode* left;
        BinNode* right;

        BinNode(string value, BinNode *leftn = NULL, BinNode *rightn = NULL){
            if(value=="and"||value=="AND")
                node_type = "AND";
            else if(value=="or"||value=="OR")
                node_type = "OR";
            else{
                negated = false;
                index = -1; //None
                // int index= NULL;  
                if(value[0]=='!'){
                    value = value.substr(1, value.size());
                    negated = true;
                }
                size_t pos = value.find('_');
                if(pos !=string::npos)
                {
                    string rest = value.substr(pos+1, value.size());
                    value = value.substr(0, pos);
                    pos = rest.find('_');
                    index = pos != string::npos ? stoi(rest.substr(0,pos)) : stoi(rest);
                }
                
                node_type = "ATTR";
                transform(value.begin(), value.end(), value.begin(), ::toupper);
                
                attribute = value;
            }
            
            left = leftn;
            right = rightn;
        }

        string getAttribute(){
            string return_val = "";
            string attr = "ATTR";
            if(node_type==attr){
                if(negated)
                    return_val += '!';
                return_val += attribute;
            }
            return return_val;
        }

        string getAttributeAndIndex(){
            string return_val = "";
            string attr = "ATTR";
            if(node_type==attr){
                if(negated)
                    return_val += '!';
                return_val += attribute;
                if(index != -1)
                    return_val += '_'+to_string(index);
            }
            return return_val;
        }

        string getNodeType(){
            return node_type;
        }
        
        BinNode* getLeft(){
            return left;
        }

        BinNode* getRight(){
            return right;
        }

        void addSubNode(BinNode *leftn, BinNode *rightn){
            left = leftn;
            right = rightn;
        }
};

class PolicyParser{
    public:
        bool verbose;
        PolicyParser(bool v = false){
            verbose = v;        
        }

        vector<string> get_tokens(string line){
            bool negated = false;
            char ch;
            int i= 0, level= 0;
            string attr;
            vector<string> tokens;
            do{
                ch = line[i];
                if(ch=='(')
                    tokens.push_back("(");
                else if(ch==')')
                    tokens.push_back(")");
                else if((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9') || ch=='!'){
                    if((ch=='a' || ch=='A') && (line[i+1]=='n' || line[i+1]=='N') && (line[i+2]=='d' || line[i+2]=='D')){
                            tokens.push_back("AND");
                            i+=2;
                            level++;
                    }
                    else if((ch=='o' || ch=='O') && (line[i+1]=='r' || line[i+1]=='R')){
                            tokens.push_back("OR");
                            i+=1;
                            level++;
                    }
                    else{
                        attr = ch;
                        i++;
                        bool parenEnd = false;
                        while( ((ch = line[i])!=')') && ch!=' ' && ch!= '\n' ){
                            attr.append(1,ch);
                            i++;
                            if(line[i]==')'){
                                parenEnd = true;
                            }
                        }
                        tokens.push_back(attr);
                        if(parenEnd)
                            tokens.push_back(")");

                        negated = false;
                    }
                }
                else if(ch==' ')	;
                i++;
            }while(line[i]);
            return tokens;
        }

        BinNode* S(int* index, vector<string> tokens){
            cout<<"S "<<*index<<" "<<tokens[*index]<<endl;
            BinNode *left = T(index, tokens);
            if(*index < tokens.size() && tokens[*index]=="OR"){
                (*index)++;
                BinNode *right = S(index, tokens);
                return new BinNode("OR", left, right);
            }

            return left;
        }

        BinNode* T(int* index, vector<string> tokens){
            cout<<"T "<<*index<<" "<<tokens[*index]<<endl;
            BinNode *left = F(index, tokens);
            if(*index < tokens.size() && tokens[*index]=="AND"){
                (*index)++;
                BinNode *right = T(index, tokens);
                return new BinNode("AND", left, right);
            }

            return left;
        }

        BinNode* F(int* index, vector<string> tokens){
            cout<<"F "<<*index<<" "<<tokens[*index]<<endl;
            string token = tokens[*index];
            (*index)++;
            BinNode *node;
            if(token=="("){
                node = S(index, tokens);
                (*index)++;
            }
            else
                node = new BinNode(token);
            return node;
        }

        BinNode* parse(string line){
            /*
                S → T '+' S | T
                T → F '.' T | F
                F → A | '('S')'
                A → 'a' | 'b' | 'c' | ... | ɛ
            */
            vector<string> tokens = get_tokens(line);
            int index = 0;
            return S(&index, tokens);
        }

        void findDuplicates(BinNode* tree, unordered_map<string,int> &dict){
            if(tree->left)
                findDuplicates(tree->left, dict);
            if(tree->right)
                findDuplicates(tree->right, dict);
            string attr = "ATTR";
            if(tree->node_type == attr){
                string key = tree->getAttribute();
                dict[key] = dict.find(key) == dict.end() ? 1 : dict[key]+1;
            }
        }
        
        void labelDuplicates(BinNode* tree, unordered_map<string,int> &dictLabel){
            if(tree->left)
                labelDuplicates(tree->left, dictLabel);
            if(tree->right)
                labelDuplicates(tree->right, dictLabel);
            string attr = "ATTR";
            if(tree->node_type == attr){
                string key = tree->getAttribute();
                if(dictLabel.find(key) == dictLabel.end()){
                    tree->index = dictLabel[key];
                    dictLabel[key]++;
                }
            }
        }
};



class Schemebase{
    public:
        unordered_map<string, string> properties;

    protected:
        bool setProperty(string scheme = ""){
            if(scheme.compare("")){
                properties["scheme"] = scheme;
            }
            return true;
        }

        unordered_map<string, string> getProperty(){
            return properties;
        }
};

class ABEnc: public Schemebase{
    unordered_map<string, int> baseSecDefs;
    ABEnc(){
        setProperty("ABEnc");
        baseSecDefs = {
            {"IND_AB_CPA", 0},
            {"IND_AB_CCA", 1},
            {"sIND_AB_CPA", 2},
            {"sIND_AB_CCA", 3}
        };
    }
};

class MSP{
    public:
        int len_longest_row;

        MSP(){
            len_longest_row = 1;
        }

        BinNode* createPolicy(string policy_string){
            PolicyParser parser = PolicyParser();
            BinNode *policy_obj = parser.parse(policy_string);
            unordered_map<string,int> dictCount, dictLabel;
            parser.findDuplicates(policy_obj, dictCount);
            for(auto i:dictCount)
                if(i.second>1)
                    dictLabel[i.first] = 0;
            parser.labelDuplicates(policy_obj, dictLabel);
            return policy_obj;
        }

        unordered_map<string, vector<int>> convertPolicyToMSP(BinNode *tree){
            vector<int> root_vector(1,1);
            len_longest_row = 1;
            return convertPolicyToMSP_Helper(tree, root_vector);
        }
    
    protected:
        unordered_map<string, vector<int>> convertPolicyToMSP_Helper(BinNode *subtree, vector<int> &curr_vector){
            unordered_map<string, vector<int>> um;
            if(!subtree)
                return um;
            
            string type = subtree->getNodeType();
            string attr = "ATTR";
            string or_node = "OR";
            string and_node = "AND";
            if(type == attr){
                um[subtree->getAttributeAndIndex()] = curr_vector;
                return um;
            }
            else if(type == or_node){
                unordered_map<string, vector<int>> left_dict, right_dict;
                left_dict = convertPolicyToMSP_Helper(subtree->getLeft(), curr_vector);
                right_dict = convertPolicyToMSP_Helper(subtree->getRight(), curr_vector);
                left_dict.insert(right_dict.begin(), right_dict.end());
                return left_dict;
            }
            else if(type == and_node){
                int length = curr_vector.size();
                vector<int> left_vector = curr_vector;
                for(int i=0; i<len_longest_row-length; i++)
                    left_vector.push_back(0);
                left_vector.push_back(1);
                vector<int> right_vector(len_longest_row,0);
                right_vector.push_back(-1);
                len_longest_row++;
                unordered_map<string, vector<int>> left_dict, right_dict;
                left_dict = convertPolicyToMSP_Helper(subtree->getLeft(), curr_vector);
                right_dict = convertPolicyToMSP_Helper(subtree->getRight(), curr_vector);
                left_dict.insert(right_dict.begin(), right_dict.end());
                return left_dict;
            }
        }
};

#endif