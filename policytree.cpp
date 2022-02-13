// C header files
#include <string>
#include <unordered_map>

// User-defined header files
#include "class_definitions.h"
#include "msp_functions.h"

using namespace std;

// unordered_map<string,int> OpType = {
//     {"OR",0},
//     {"AND",1},
//     {"ATTR",2},
//     {"THRESHOLD",3},
//     {"CONDITIONAL",4},
//     {"NONE",5}
// };

int main(){
    string line= "((ONE and THREE) and (TWO OR FOUR))";
	PolicyParser parser = PolicyParser();
    BinNode *root = parser.parse(line);
    walkThrough(root, "", true);
    MSP msp = MSP();
    msp.createPolicy(line);
}