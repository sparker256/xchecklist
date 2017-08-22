#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include "../src/interface.h"
#include "../src/speech.h"
#include "../src/utils.h"
#include "../src/chkl_parser.h"

bool voice_state = true;


struct dref_s{
  dref_s(int t, std::vector<double> *v) : type(t), values(v){};
  int type;
  int index;
  std::vector<double> *values;    
};

typedef std::map<std::string, struct dref_s *> datarefs_map_t;

datarefs_map_t datarefs_map;


struct action_s{
  struct dref_s *dref;
  int index;
  double value;
  action_s(struct dref_s *d, int i, double v): dref(d), index(i), value(v) {};
};

//Actions to be performed "simultaneously" (same step)
typedef std::vector<action_s *> action_list_t;
//Action list to be performed at successive steps 
typedef std::vector<action_list_t *> actions_list_t;
//Action lists for different items to test
typedef std::map<std::string, actions_list_t *> regres_steps_t;

regres_steps_t regres_steps;

std::string get_tag(std::string text)
{
  size_t beg, end;

  if((beg = text.find_first_of('<')) == text.npos){
    return "";
  }
  if((end = text.find_first_of('>', beg)) == text.npos){
    return "";
  }
  return text.substr(beg + 1, end - beg - 1);
}


bool dispose_dataref(dataref_p *dref)
{
  (void)dref;
  return true;
}

double get_double_dataref(dataref_p dref)
{
  if(dref->dref){
    dref_s *tmp = (dref_s *)dref->dref;
    //std::cout << "Double dref: " << tmp->values->at(0) << std::endl;
    return tmp->values->at(0);
  }
  //std::cout << "Double dref fallback: 3.14" << std::endl;
  return 3.141592657;
}

double get_float_dataref(dataref_p dref)
{
  if(dref->dref){
    dref_s *tmp = (dref_s *)dref->dref;
    //std::cout << "Float dref: " << tmp->values->at(0) << std::endl;
    return tmp->values->at(0);
  }
  //std::cout << "Float dref fallback: 3.14" << std::endl;
  return 3.14;
}

double get_int_dataref(dataref_p dref)
{
  if(dref->dref){
    dref_s *tmp = (dref_s *)dref->dref;
    //std::cout << "Int dref: " << (int)tmp->values->at(0) << std::endl;
    return (int)tmp->values->at(0);
  }
  //std::cout << "Int dref fallback: 442" << std::endl;
  return 442;
}

double get_float_array_dataref(dataref_p dref)
{
  if(dref->dref){
    dref_s *tmp = (dref_s *)dref->dref;
    //std::cout << "Float array dref[" << dref->index << "]: " << (float)tmp->values->at(dref->index) << std::endl;
    return (int)tmp->values->at(dref->index);
  }
  //std::cout << "Float array dref fallback: 7.77" << std::endl;
  return 7.77;
}

double get_int_array_dataref(dataref_p dref)
{
  if(dref->dref){
    dref_s *tmp = (dref_s *)dref->dref;
    //std::cout << "Int array dref[" << tmp->index << "]: " << (int)tmp->values->at(tmp->index) << std::endl;
    return (int)tmp->values->at(tmp->index);
  }
  //std::cout << "Int array dref fallback: 7.77" << std::endl;
  return 14;
}



bool find_array_dataref(const char *name, int index, dataref_p *dref, value_type_t preferred_type)
{
  //std::cout << "Find array dataref: " << name << "[" << index << "], of type " << preferred_type << std::endl;

  *dref = (dataref_p)malloc(sizeof(struct dataref_struct_t));
  datarefs_map_t::iterator i = datarefs_map.find(std::string(name));
  if(i == datarefs_map.end()){
    //Carenado style?
    return false;
  }
  (*dref)->dref = /*(XPLMDataRef)*/i->second;
  (*dref)->index = index;

  switch(preferred_type){
    case TYPE_INT:
      (*dref)->accessor = get_int_array_dataref;
      break;
    default:
      (*dref)->accessor = get_float_array_dataref;
      break;
  }
  return true;
}

bool find_dataref(const char *name, dataref_p *dref, value_type_t preferred_type)
{
  *dref = (dataref_p)malloc(sizeof(struct dataref_struct_t));
  datarefs_map_t::iterator i = datarefs_map.find(std::string(name));
  if(i != datarefs_map.end()){
    (*dref)->dref = /*(XPLMDataRef)*/i->second;
  }

  switch(preferred_type){
    case TYPE_INT:
      (*dref)->accessor = get_int_dataref;
      break;
    case TYPE_FLOAT:
      (*dref)->accessor = get_float_dataref;
      break;
    default:
      (*dref)->accessor = get_double_dataref;
      break;
  }
  return true;
}

class my_checklist{
  std::vector<checklist_item_desc_t> items_list;
  size_t maxlen;
  int activated;
  regres_steps_t::iterator rsi;
  actions_list_t::iterator lists;
  action_list_t::iterator list;
  bool process_datarefs;
  bool should_trigger;
  int safety;
  int problems;
  bool manual;
  std::string title;

 public:
  my_checklist(const std::string t, int size, checklist_item_desc_t items[]);
  void activate_item(int item);
  bool check_item();
  bool check_item(int itemNo);
  //bool item_active(){a = activated; return !(activated < 0);};
  void notify_checked(int itemNo);
  bool work(bool visible = true);
  int get_problems(){return problems;};
  std::string active_item_text(){return (activated < 0) ? "" : items_list[activated].text;};
  const std::string &get_title(){return title;};
};

bool my_checklist::work(bool visible)
{
  //std::cout << "Working... " << problems << " problems so far..." << std::endl;
  --safety;
  if(safety == 0){
    std::cout << std::endl << " * Error * Safety triggered, checking \"manually\"." << std::endl;
    manual = true;
    ++problems;
  }
  if(process_datarefs){
    while(list != (*lists)->end()){
      struct action_s *a = *list; 
      a->dref->values->at(a->index) = a->value;
      //std::cout << std::endl << "    -> " << a->value << std::endl;
      ++list;
    }
    ++lists;
    if(lists == rsi->second->end()){
      should_trigger = true;
      process_datarefs = false;
    }else{
      list = (*lists)->begin();
    }
  }else if((activated >= 0) && manual){
    check_item();
    should_trigger = true;
  }
  do_processing(visible, true);
  bool switch_next = false;
  return checklist_finished(&switch_next);
}


my_checklist::my_checklist(const std::string t, int size, checklist_item_desc_t items[]) : maxlen(0), 
                        activated(-1), process_datarefs(false), 
                        should_trigger(false), safety(10), problems(0), manual(false), title(t)
{
  for(int i = 0; i < size; ++i){
    items_list.push_back(items[i]);
    if(strlen(items[i].text) > maxlen){
      maxlen = strlen(items[i].text);
    }
  }
}

void my_checklist::notify_checked(int itemNo)
{
  if(itemNo != activated){
    std::cout << std::endl << " * Error * " << activated << "active while checked " << itemNo << "." << std::endl;
    ++problems;
    return;
  }
  if(!should_trigger){
    std::cout << std::endl << " * Error * Item number " << itemNo << " checked unexpectedly." << std::endl;
    ++problems;
    return;
  }
  activated = -1;
  process_datarefs = false;
  safety = 10;
  std::string space(maxlen - strlen(items_list[itemNo].text) + 3, '.');
  std::cout << space << items_list[itemNo].suffix << std::endl;
  //std::cout << "Checked item number " << itemNo << std::endl;
}

void my_checklist::activate_item(int item)
{
  std::string text(items_list[item].text);
  activated = item;
  std::cout << "  " << text;
  should_trigger = false;
  manual = false;
  std::string tag = get_tag(text);
  if(tag != ""){
    rsi = regres_steps.find(tag);
    if(rsi == regres_steps.end()){
      // unknown tag => no receipt to activate
      //std::cout << "Have tag, but no receipt, checking manually." << std::endl;
      manual = true;
    }else{
      lists = rsi->second->begin();
      list = (*lists)->begin();
      process_datarefs = true;
    }
  }else{
    // no tag => no receipt to activate
    //std::cout << "No tag, checking manually." << std::endl;
    manual = true;
  }
  safety = 10;
}

bool my_checklist::check_item()
{
  if(activated < 0){
    std::cout << std::endl << " * Error * No item active!" << std::endl;
    return false;
  }
  if(item_checked(activated)){
    //std::string text(items_list[activated].text);
    //std::string space(maxlen - text.length() + 3, 'x');
    //std::cout << space << items_list[activated].suffix << std::endl;
    return true;
  }
  return false;
}

bool my_checklist::check_item(int itemNo)
{
  if(itemNo != activated){
    std::cout << std::endl << " * Error * Request to check inactive item (active " << activated << ", requested " << itemNo << ")!" << std::endl;
    return false;
  }
  return this->check_item();
}


my_checklist *cl;

bool create_checklist(unsigned int size, const char *title,
                      checklist_item_desc_t items[], int width,
                      int index, int force_show)
{
  (void)size;
  (void)title;
  (void)items;
  (void)width;
  (void)index;
  (void)force_show;

  if(cl){
    delete cl;
  }
  cl = new my_checklist(title, size, items);

  return true;
}



bool activate_item(int itemNo)
{
  (void)itemNo;
  //std::cout << "Item " << itemNo << " activated" << std::endl;
  cl->activate_item(itemNo);
  return true;
}

bool check_item(int itemNo)
{
  (void)itemNo;
  //std::cout << "Item " << itemNo << " checked" << std::endl;
  //cl->check_item(itemNo);
  cl->notify_checked(itemNo);
  return true;
}

bool init_speech(){return true;}
void say(const char *text){(void)text;}
void close_speech(){}
void cleanup_speech(){}
bool speech_active(){return true;}
bool spoken(float elapsed){(void)elapsed;return true;}

bool walkthrough_checklists()
{
  int problems = 0;
  while(1){
    bool switch_next = false;
    std::cout << cl->get_title() << std::endl;
    while(!checklist_finished(&switch_next)){
      cl->work();
    }
    //to allow sw_show
    if(cl->work(false)){
      problems += cl->get_problems();
      if(!next_checklist(false)){
        break;
      }
    }
  }
  std::cout << "Encountered " << problems << " problems." << std::endl;
  return true;
}

struct action_s *read_action(std::string &line, size_t start, size_t stop)
{
  std::string name("");
  double val;
  //std::cout << "Action: " << line.substr(start, stop) << std::endl;
  std::istringstream action_str(line.substr(start, stop));
  action_str >> name >> val;
  if(name == ""){
    return NULL;
  }
  datarefs_map_t::iterator i = datarefs_map.find(name);
  if(i == datarefs_map.end()){
    size_t pos;
    int value = 0;
    if((pos = name.find("[")) != name.npos){
      std::string base_name = name.substr(0, pos);
      std::istringstream idx_str(name.substr(pos + 1));
      idx_str >> value;
      //std::cout << "Array: " << base_name << "[" << value << "] = " << val << std::endl;
      i = datarefs_map.find(base_name);
      if(i == datarefs_map.end()){
        std::cout << "Can't find dataref " << base_name << std::endl;
        return NULL;
      }
      return new struct action_s(i->second, value, val);
    }
    return NULL;
  }
  //std::cout << "Action created!" << val << std::endl;
  return new struct action_s(i->second, 0, val);
}


void read_comments(const char *fname)
{
  std::ifstream infile(fname);
  std::string line, comment, type, name;
  double val;
  while(std::getline(infile, line)){
    size_t pos;
    if((pos = line.find("#?#")) != line.npos){
      //Dataref definitions
      comment = line.substr(pos + 3);
      std::istringstream dref_str(comment);
      dref_str >> type >> name;
      //std::cout << "Type: " << type << " Name: " << name <<std::endl;
      int d_t = XC_UNKNOWN;
      if(type.find_first_of('i') != type.npos){
        d_t |= XC_INTEGER;
      }
      if(type.find_first_of('f') != type.npos){
        d_t |= XC_FLOAT;
      }
      if(type.find_first_of('d') != type.npos){
        d_t |= XC_DOUBLE;
      }
      if(type.find_first_of('I') != type.npos){
        d_t |= XC_INT_ARRAY;
      }
      if(type.find_first_of('F') != type.npos){
        d_t |= XC_FLOAT_ARRAY;
      }
      std::vector<double> *values = new std::vector<double>();
      while(dref_str.good()){
        dref_str >> val;
        values->push_back(val);
      }
      //std::cout << "Creating dataref " << name << std::endl;
      datarefs_map.insert(std::pair<std::string, dref_s *>(std::string(name), new dref_s(d_t, values)));
    }else if((pos = line.find("#!#")) != line.npos){
      //Action defs
      pos += 3;
      std::string tag = get_tag(line);
      if(tag == ""){
        //std::cout << "Found action list, but no tag!" << std::endl;
        continue;
      }
      pos += tag.length() + 2;
      size_t end_pos;
      actions_list_t *actions_list = NULL;
      action_list_t *actions = NULL;
      while(1){
        end_pos = line.find_first_of(",:", pos);
        struct action_s *action = read_action(line, pos, end_pos);
        if(action != NULL){
          if(actions == NULL){
            actions = new action_list_t();
          }
          actions->push_back(action);
          if(line[end_pos] == ':'){
            if(actions_list == NULL){
              actions_list = new actions_list_t();
            }
            actions_list->push_back(actions);
            actions = NULL;
          }
        }
        action = NULL;
        if(end_pos == line.npos){
          break;
        }
        pos = end_pos + 1;
        //std::cout << "pos: " << pos << std::endl;
      }
      if((actions != NULL) && (actions_list == NULL)){
        actions_list = new actions_list_t();
      }
      if(actions != NULL){
        actions_list->push_back(actions);
      }
      if(actions_list != NULL){
        regres_steps.insert(std::pair<std::string, actions_list_t *>(tag, actions_list));
      }
    }
  }
}

int main(int argc, char *argv[])
{
  int hidden_param = 0;
  if(argc > 2){
    hidden_param = atoi(argv[2]);
    read_comments(argv[1]);
    start_checklists(argv[1], hidden_param);
  }else if(argc > 1){
    start_checklists(argv[1], 0);
  }else{
    std::cout << "Usage: " << argv[0] << " checklist_name" << std::endl;
    std::cout << "  reads the checklist and reports any errors encountered." << std::endl;
    std::cout << std::endl;
    std::cout << "For developement/debugging purposes a second agrument can be specified." << std::endl;
    std::cout << "  Its value is treated as a bit field with the following meaning:" << std::endl;
    std::cout << "    bit 0 - activates verbose mode of a parser" << std::endl;
    std::cout << "    bit 1 - writes out all the expressions found in a checklist" << std::endl;
    std::cout << "            and tries to evaluate them" << std::endl;
    std::cout << "    bit 2 - enables checklist walkthrough mode; used for regression testing" << std::endl;
  }

  if(hidden_param & 4){
    std::cout << "================================================" << std::endl;
    std::cout << "Checklist printout" << std::endl;
    print_checklists();
    int all, msize;
    constname_t *names; 
    int *indexes;
    if(get_checklist_names(&all, &msize, &names, &indexes)){
      std::cout << "================================================" << std::endl;
      std::cout << "Available " << msize << " named checklists out of " << all << std::endl;
      for(int i = 0; i < msize; ++i){
        std::cout << "  " << names[i] << std::endl;
      }
      free_checklist_names(all, msize, &names, &indexes);

    }
    std::cout << "================================================" << std::endl;
    std::cout << "Checklist walkthrough" << std::endl;
    walkthrough_checklists();
  }

  stop_checklists();
  //discard_checklist();
  xcClose();
  return 0;
}

