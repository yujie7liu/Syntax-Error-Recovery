/* Complete recursive descent parser for the calculator language.
    Builds on figure 2.16.  Prints a trace of productions predicted and
    tokens matched.  Does no error recovery: prints "syntax error" and
    dies on invalid input.
    Michael L. Scott, 2008-2017.
*/
#include <stdexcept>
#include <iostream>
#include <ios>
#include "scan.h"
#include <cstdlib>
#include <vector>
#include <string>
//using this instead of namespace std to avoid allocating for those unused
using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::noskipws;

const char* names[] = {"check", "read", "write", "id", "literal", "gets", "if",
"fi", "do", "od", "equal", "notequal", "smaller",
"greater", "smallerequal","greaterequal",
"add", "sub", "mul", "div", "lparen", "rparen", "eof", "eps"};

static token input_token;
static int tabNum = 0;
static int hasError = 0;
static token s_follow[] = {t_id, t_read, t_write, t_if, t_do, t_check, t_eof};
static token r_follow[] = {t_id, t_read, t_write, t_if, t_do, t_check, t_eof, t_fi, t_rparen};
static token e_follow[] = {t_id, t_read, t_write, t_if, t_do, t_check, t_eof, t_fi, t_rparen
                        , t_equal, t_notequal, t_smaller, t_greater, t_smallerequal, t_greaterequal};
static string buffer = ""; //Keep track of the nonterminal currently being parsed
string postIndent(string str, int tab){
  for(int i = 0; i <= tab; i++){
    str += " ";
  }
  str += std::to_string(tab);
  return str;
}
string preIndent(string str, int tab){
  for(int i = 0; i <= tab; i++){
    str = " " + str;
  }
  return str;
}
string prefix(string str, string tail){
  if(tail == "") return str;
  for (int i = 0; i < tail.length(); ++i){
    if(tail[i] == ' '){
      return tail.substr(0,i)+" "+ str +" "+ tail.substr(i+1, tail.length() - i);
    }
  }
  return "prefix error";
}

int contains(token t, token set[]){
  int i = 0;
  while(set[i]){
    if (t == set[i++]) {
      return 1;
    }
  }
  cout << names[t] << " consumed" << endl;
  return 0;
}

void error () {
    cout << "Syntax error" << endl;
    exit (1);
}

string match (token expected) {
    if (input_token == expected) {
        //cout << "Matched "<<names[expected] << endl;
        input_token = scan ();
        //cout << "match next: "<<names[input_token] << endl;
    }
    else{
      cout << "Token " << names[input_token] << ": do you mean " << names[expected] << "?"<< endl;
      throw string("match");
  }

  return "";
}



string program ();
string stmt_list ();
string stmt ();
string expr ();
string expr_tail();
string term_tail ();
string term ();
string factor_tail ();
string factor ();
string relation_op();
string add_op ();
string mul_op ();
string relation();

string program () {
    cout << "P: input token: " << names[input_token] << endl;
    try{
        switch (input_token) {
            case t_id:
            case t_read:
            case t_write:
            case t_if:
            case t_do:
            case t_check:
            case t_eof:{
              tabNum++;
                cout << "predict program -->stmt_list eof" << endl;
                string str1 = "(program \n" ;
                str1 = postIndent(str1, tabNum);
                str1 += "[";

                str1 += stmt_list ();
                match (t_eof);
                str1 = postIndent(str1, tabNum);
                str1 += "]\n";
                if(hasError) return "";
                return str1+")\n";
            }
            default:
            //cout << "program wrong\n";
            throw string("program");
            return "";
        }
    }catch(string e){
        cout << "Not expecting " << names[input_token] << " in Program"<< endl;
        return "";
    }
}

string stmt_list () {
  cout << "SL: input token: " << names[input_token] << endl;
  switch (input_token) {
      case t_id:
      case t_check:
      case t_write:
      case t_read:
      case t_if:
      case t_do:{
        string str1 = "";
        str1 = postIndent(str1, tabNum);
        str1 += "("+stmt();
        str1 += stmt_list();
        str1 = postIndent(str1, tabNum);
        str1 += ") sl\n";

        tabNum--;
        return str1;
      }
      case t_eof:
        tabNum--;
        return "\n";          /*  epsilon production */
    default:
      tabNum--;
      return "\n";
    }
}

string stmt () {
  cout << "S: input token: " << names[input_token] << endl;
  tabNum++;
  try{
    switch (input_token) {
        case t_id:{
            match (t_id);
            match (t_gets);
            string str1 = "( := (id "+getImage() +")" + relation();//Used to be expr()
            tabNum--;
          return str1;
      }
        case t_read:
        match (t_read);
        match (t_id);
        tabNum--;
        return "(read id)\n";
      case t_write:{
        match (t_write);
            string str1 = relation();//Used to be expr()
            str1 = postIndent(str1, tabNum);
          tabNum--;
          return "(write " + str1 + ")write\n";
        }
        case t_if:{
          match(t_if);
          string str1 = "(if \n";
          str1 = postIndent(str1, tabNum);
          str1 += relation();
          str1 = postIndent(str1, tabNum);
          string str2  = stmt_list();
          str2 = postIndent(str2, tabNum);
          match(t_fi);
          tabNum--;
          return str1 +"if[\n"+ str2 + "]) fi\n";
        }
        case t_do:{
          match(t_do);
          string str1 = "(do\n";
          str1 += stmt_list();
          str1 = postIndent(str1, tabNum);
          match(t_od);
          tabNum--;
          return "do["+ str1 + "]) od\n";
        }
        case t_check:{
          match(t_check);
          string str1 = "";
          
          str1 = postIndent(str1, tabNum);
          str1 += relation();
          str1 = postIndent(str1, tabNum);
          tabNum--;
          return "(check\n"+str1+")check\n";
        }
        default: //SyntaxErrorException e; throw e; //Throw the exception
        //cout << "statement wrong\n";
        error();
        tabNum--;
        return "";
      }
    }catch(string e){
      if(e == "match") cout <<"Not expecting " << names[input_token] << " in Statement" <<endl;
      else cout << "Not expecting " << names[input_token] << " in " << e << endl;
      cout << "deleted: " << names[input_token] << endl;
      input_token = scan();
      //cout << "next: " << names[input_token] << endl;
      while(!contains(input_token, s_follow)
            &&input_token != t_eof){
            cout << "deleted: " << names[input_token] << endl;
            input_token = scan();

    }
    if(contains(input_token, s_follow)){
        hasError = 1;
        cout << "follow token "<< names[input_token]<<" found" << endl;
        return "(error)\n";
    }else{} //If having reached eof
            return "";
        }
}

string expr () {
  cout << "E: input token: " << names[input_token] << endl;
  tabNum++;
  try{
    string str1 = term ();
    string str2 = term_tail ();
    tabNum--;
    return prefix(str1, str2);
    }catch(string e){
      if(e == "match") cout <<"Not expecting " << names[input_token] << " in expression" <<endl;
      else cout << "Not expecting " << names[input_token] << " in " << e << endl;
      cout << "deleted: " << names[input_token] << endl;
            while(!contains(input_token, e_follow)
            &&input_token != t_eof){
            cout << "deleted: " << names[input_token] << endl;
            input_token = scan();

       }
        if(contains(input_token, e_follow)){
            hasError = 1;
            tabNum--;
            cout << "follow token "<< names[input_token]<<" found" << endl;
            return "(error)\n";
        }else{

            } //If having reached eof
            tabNum--;
            return "";
        }
  error ();
  tabNum--;
  return "";

}
// the new built method by us

string expr_tail(){
  cout << "ET: input token: " << names[input_token] << endl;
  tabNum++;
  switch (input_token) {
    case t_equal:
    case t_notequal:
    case t_smaller:
    case t_greater:
    case t_smallerequal:
    case t_greaterequal:{
        string str1 = relation_op();
        string str2 = expr();
        tabNum--;
        return str1+" "+str2;
    }
    case t_id:
    case t_read:
    case t_write:
    case t_eof:
    tabNum--;
    return "";
    default:
    tabNum--;
    return "";
        //predict set ET -> epsilon = SL
}
}

string term_tail () {
  cout << "TT: input token: " << names[input_token] << endl;
  tabNum++;
  cout << "input token: " << names[input_token] << endl;  switch (input_token) {
    case t_add:
    case t_sub:{
        string str1 = add_op ();
        str1 += " ";
        str1 += term ();
        string str2 = term_tail ();
        tabNum--;
        return prefix(str1,str2);
        //return " "+str1+" ";
    }
    case t_rparen:
    case t_id:
    case t_read:
    case t_write:
    case t_eof:
      tabNum--;
      return "";          /*  epsilon production */
    default:
      return "";
}
}

string term () {
  try{
    cout << "T: input token: " << names[input_token] << endl;
  tabNum++;
  string str1 = factor ();
  string str2 = factor_tail ();
  tabNum--;
  //return "("+str1+str2+")";
  return prefix(str1, str2);
  cout << "term wrong\n";
}catch(string e){
  throw string("term");
  tabNum--;
}
  return "";

}

string factor_tail () {
  cout << "FT: input token: " << names[input_token] << endl;
  tabNum++;
  switch (input_token) {
    case t_mul:
    case t_div:{
        string str1 = mul_op ();
        //str1 += " ";
        string str2 = factor ();
        str1 += str2;
        str1 += factor_tail ();
        tabNum--;
        return str1+"";
    }
    case t_add:
    case t_sub:
    case t_rparen:
    case t_id:
    case t_read:
    case t_write:
    case t_eof:
      tabNum--;
      return "";          /*  epsilon production */
    default:
      return "";
    }
}

string factor () {
  cout << "F: input token: " << names[input_token] << endl;
  tabNum++;
  switch (input_token) {
    case t_id :{
      match (t_id);
      tabNum--;
      string str1 = "(id"+getImage()+")";
      return str1;
    }
    case t_literal:{
      match (t_literal);
      tabNum--;
      string str1 = "lit"+getImage();
      return str1;
    }
    case t_lparen:{
    match (t_lparen);
    string str1 = relation ();
    match (t_rparen);
    tabNum--;
    return "("+str1+")";
  }
    default:
    cout << "factor wrong\n";
    throw string("factor");
    tabNum--;
    return "";
}
}
// the new built one
string relation_op(){
  cout << "RO: input token: " << names[input_token] << endl;
  tabNum++;
  switch(input_token){
    case t_equal:
    match(t_equal);
    tabNum--;
    return "== ";
    case t_notequal:
    match(t_notequal);
    tabNum--;
    return "<> ";
    case t_smaller:
    match(t_smaller);
    tabNum--;
    return "< ";
    case t_greater:
    match(t_greater);
    tabNum--;
    return "> ";
    case t_smallerequal:
    match(t_smallerequal);
    tabNum--;
    return "<= ";
    case t_greaterequal:
    match(t_greaterequal);
    tabNum--;
    return ">= ";
    default:
    cout << "relatioon op wrong\n";
    throw string("relation operation");
    tabNum--;
    return "";
}
}

string add_op () {
  cout << "AO: input token: " << names[input_token] << endl;
  tabNum++;
  switch (input_token) {
    case t_add:
    match (t_add);
    tabNum--;
    return "+ ";
    case t_sub:
    match (t_sub);
    tabNum--;
    return "- ";
    default:
    cout << "add op wrong\n";
    throw string("arithmetic operator");
    tabNum--;
    return "";
}
}

string mul_op () {
  tabNum++;
  cout << "MO: input token: " << names[input_token] << endl;  switch (input_token) {
    case t_mul:
    printf ("predict mul_op --> mul\n");
    match (t_mul);
    tabNum--;
    return "* ";
    case t_div:
    printf ("predict mul_op --> div\n");
    match (t_div);
    tabNum--;
    return "/ ";
    default:
    tabNum--;
    cout << "mul op wrong\n";
    throw string("multiplication operator");
    return "";
}
}

string relation(){
  cout << "R: input token: " << names[input_token] << endl;
    try{
      tabNum++;
      string str2 = expr();
      string str1 = expr_tail();
      tabNum--;
      return "("+prefix(str2, str1)+ ")\n";
    }catch(string e){
      if(e == "match") cout <<"Not expecting " << names[input_token] << " in relation" <<endl;
      else cout << "Not expecting " << names[input_token] << " in " << e << endl;
      input_token = scan();
      cout << "deleted: " << names[input_token] << endl;
            while(!contains(input_token, r_follow)&&input_token != t_eof){
            cout << "deleted: " << names[input_token] << endl;
            input_token = scan();
            cout << input_token<<endl;
        }
        if(contains(input_token, r_follow)){
            hasError = 1;
            tabNum--;
            cout << "follow token "<<names[input_token]<<" found" << endl;
            return "(error)\n";
        }else{} //If having reached eof
        tabNum--;
        return " eof";
      } 
}

int main () {
    input_token = scan ();
    cout << program ();
    cout << prefix("23", "* 54");
    return 0;
}
