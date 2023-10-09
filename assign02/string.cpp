#include "string.h"
#include "value.h"
#include <cstring>
#include <iostream>
String::String(const std::vector<std::string> &sub_string)
  : ValRep(VALREP_STRING)
  , sub_string(sub_string) {
}

String::String(std::string &formatted_string)
  : ValRep(VALREP_STRING)
  , sub_string(format_string(formatted_string)) {
}

String::~String() {
}

std::vector<std::string> &String::get_sub_string(){
    return sub_string;
}

unsigned String::get_sub_string_len(){
    return sub_string.size();
}

std::vector<std::string> String::format_string(std::string non_format_string){
    std::vector<std::string> fixed_string;
        unsigned i = 0;
        unsigned string_size = non_format_string.size() - 1;
        if(non_format_string.size() == 0) {
            return fixed_string;
        }
        while(i < string_size) {
            if(i + 1 < string_size) {
                std::string special_token;
                special_token = special_token + non_format_string[i] + non_format_string[i + 1];
                if(special_token == "\\n" || special_token == "\\r" || special_token == "\\t" 
                || (non_format_string[i] == 92 && non_format_string[i + 1] == '"')) {
                    if(special_token == "\\n"){
                        fixed_string.push_back("\n");
                    } else if (special_token == "\\r"){
                        fixed_string.push_back("\r");
                    } else if(special_token == "\\t"){
                        fixed_string.push_back("\t");
                    } else if(non_format_string[i] == 92 && non_format_string[i + 1] == '"'){
                        fixed_string.push_back("\"");
                    }
                    i += 2;
                    continue;
                } else {
                      std::string non_special_token;
                      non_special_token = non_special_token + non_format_string[i];
                      fixed_string.push_back(non_special_token);
                    i += 1;
                }
            } else {
                //final character
                std::string final_token;
                final_token = final_token + non_format_string[i];
                fixed_string.push_back(final_token);
                i += 1;
            }
        }
        fixed_string.erase(fixed_string.begin());
        return fixed_string;
        
}